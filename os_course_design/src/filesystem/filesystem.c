#include "filesystem.h"

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FS_BLOCK_SIZE 64
#define FS_BLOCK_COUNT 64
#define FS_MAX_FILE_BLOCKS 8
#define FS_MAX_CHILDREN 16

typedef enum {
    NODE_FILE = 0,
    NODE_DIR = 1
} NodeType;

typedef struct FsNode {
    char name[MAX_NAME_LEN];
    NodeType type;
    struct FsNode *parent;
    struct FsNode *children[FS_MAX_CHILDREN];
    int child_count;
    int size;
    int blocks[FS_MAX_FILE_BLOCKS];
    int block_count;
    char *content;
} FsNode;

static unsigned char fs_bitmap[FS_BLOCK_COUNT];
static FsNode *fs_root = NULL;
static FsNode *fs_current = NULL;

static FsNode *create_node(const char *name, NodeType type, FsNode *parent) {
    FsNode *node = (FsNode *)calloc(1U, sizeof(FsNode));
    if (node == NULL) {
        return NULL;
    }
    strncpy(node->name, name, MAX_NAME_LEN - 1);
    node->type = type;
    node->parent = parent;
    for (int i = 0; i < FS_MAX_FILE_BLOCKS; ++i) {
        node->blocks[i] = -1;
    }
    return node;
}

static void init_filesystem(void) {
    if (fs_root != NULL) {
        return;
    }
    memset(fs_bitmap, 0, sizeof(fs_bitmap));
    fs_root = create_node("/", NODE_DIR, NULL);
    fs_current = fs_root;
}

static void free_node(FsNode *node) {
    if (node == NULL) {
        return;
    }
    for (int i = 0; i < node->child_count; ++i) {
        free_node(node->children[i]);
    }
    free(node->content);
    free(node);
}

static FsNode *find_child(FsNode *dir, const char *name) {
    if (dir == NULL || dir->type != NODE_DIR) {
        return NULL;
    }
    for (int i = 0; i < dir->child_count; ++i) {
        if (strcmp(dir->children[i]->name, name) == 0) {
            return dir->children[i];
        }
    }
    return NULL;
}

static int add_child(FsNode *dir, FsNode *child) {
    if (dir->child_count >= FS_MAX_CHILDREN) {
        return 0;
    }
    dir->children[dir->child_count++] = child;
    return 1;
}

static void remove_child(FsNode *dir, int index) {
    for (int i = index; i < dir->child_count - 1; ++i) {
        dir->children[i] = dir->children[i + 1];
    }
    dir->child_count--;
}

static int allocate_blocks(FsNode *file, int needed_blocks) {
    int allocated = 0;

    for (int i = 0; i < FS_BLOCK_COUNT && allocated < needed_blocks; ++i) {
        if (!fs_bitmap[i]) {
            fs_bitmap[i] = 1;
            file->blocks[allocated++] = i;
        }
    }

    if (allocated < needed_blocks) {
        for (int i = 0; i < allocated; ++i) {
            fs_bitmap[file->blocks[i]] = 0;
            file->blocks[i] = -1;
        }
        return 0;
    }

    file->block_count = needed_blocks;
    return 1;
}

static void release_blocks(FsNode *file) {
    for (int i = 0; i < file->block_count; ++i) {
        if (file->blocks[i] >= 0) {
            fs_bitmap[file->blocks[i]] = 0;
            file->blocks[i] = -1;
        }
    }
    file->block_count = 0;
}

static void print_bitmap(void) {
    print_divider("空闲块位图");
    for (int i = 0; i < FS_BLOCK_COUNT; ++i) {
        printf("%d", fs_bitmap[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        } else {
            printf(" ");
        }
    }
}

static void fs_mkdir(void) {
    char name[MAX_NAME_LEN];
    read_string("目录名: ", name, sizeof(name));
    if (find_child(fs_current, name) != NULL) {
        printf("同名文件或目录已存在。\n");
        return;
    }
    FsNode *dir = create_node(name, NODE_DIR, fs_current);
    if (dir == NULL || !add_child(fs_current, dir)) {
        free(dir);
        printf("创建目录失败。\n");
        return;
    }
    printf("目录创建成功。\n");
}

static void fs_create_file(void) {
    char name[MAX_NAME_LEN];
    read_string("文件名: ", name, sizeof(name));
    if (find_child(fs_current, name) != NULL) {
        printf("同名文件或目录已存在。\n");
        return;
    }
    FsNode *file = create_node(name, NODE_FILE, fs_current);
    if (file == NULL || !add_child(fs_current, file)) {
        free(file);
        printf("创建文件失败。\n");
        return;
    }
    printf("文件创建成功。\n");
}

static void fs_write_file(void) {
    char name[MAX_NAME_LEN];
    char content[MAX_INPUT_LEN];
    read_string("文件名: ", name, sizeof(name));
    FsNode *file = find_child(fs_current, name);
    if (file == NULL || file->type != NODE_FILE) {
        printf("文件不存在。\n");
        return;
    }

    read_string("写入内容(单行): ", content, sizeof(content));

    release_blocks(file);
    free(file->content);
    file->content = NULL;

    int len = (int)strlen(content);
    int needed_blocks = (len + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE;
    if (needed_blocks == 0) {
        needed_blocks = 1;
    }
    if (needed_blocks > FS_MAX_FILE_BLOCKS) {
        printf("文件过大，超过模拟文件系统限制。\n");
        return;
    }
    if (!allocate_blocks(file, needed_blocks)) {
        printf("磁盘空间不足。\n");
        return;
    }

    file->content = (char *)calloc((size_t)len + 1U, sizeof(char));
    if (file->content == NULL) {
        release_blocks(file);
        printf("写入失败。\n");
        return;
    }
    strcpy(file->content, content);
    file->size = len;
    printf("写入成功，占用块数: %d\n", file->block_count);
}

static void fs_read_file(void) {
    char name[MAX_NAME_LEN];
    read_string("文件名: ", name, sizeof(name));
    FsNode *file = find_child(fs_current, name);
    if (file == NULL || file->type != NODE_FILE) {
        printf("文件不存在。\n");
        return;
    }
    printf("文件内容: %s\n", file->content == NULL ? "(空)" : file->content);
}

static void fs_delete(void) {
    char name[MAX_NAME_LEN];
    read_string("要删除的文件/目录名: ", name, sizeof(name));
    for (int i = 0; i < fs_current->child_count; ++i) {
        FsNode *node = fs_current->children[i];
        if (strcmp(node->name, name) == 0) {
            if (node->type == NODE_DIR && node->child_count > 0) {
                printf("目录非空，不能直接删除。\n");
                return;
            }
            if (node->type == NODE_FILE) {
                release_blocks(node);
            }
            free_node(node);
            remove_child(fs_current, i);
            printf("删除成功。\n");
            return;
        }
    }
    printf("未找到目标。\n");
}

static void fs_list(void) {
    print_divider("目录内容");
    for (int i = 0; i < fs_current->child_count; ++i) {
        FsNode *node = fs_current->children[i];
        printf("%s\t%s\t大小:%d\n",
               node->type == NODE_DIR ? "[DIR]" : "[FILE]",
               node->name,
               node->size);
    }
    if (fs_current->child_count == 0) {
        printf("(空目录)\n");
    }
}

static void fs_cd(void) {
    char name[MAX_NAME_LEN];
    read_string("输入目录名(返回上级请输入 ..): ", name, sizeof(name));
    if (strcmp(name, "..") == 0) {
        if (fs_current->parent != NULL) {
            fs_current = fs_current->parent;
        }
        return;
    }

    FsNode *target = find_child(fs_current, name);
    if (target == NULL || target->type != NODE_DIR) {
        printf("目录不存在。\n");
        return;
    }
    fs_current = target;
}

static void fs_pwd_recursive(FsNode *node) {
    if (node == NULL) {
        return;
    }
    if (node->parent != NULL) {
        fs_pwd_recursive(node->parent);
        if (strcmp(node->name, "/") != 0) {
            printf("%s/", node->name);
        }
    } else {
        printf("/");
    }
}

static void fs_pwd(void) {
    printf("当前路径: ");
    fs_pwd_recursive(fs_current);
    printf("\n");
}

void filesystem_menu(void) {
    int choice = -1;
    init_filesystem();

    while (choice != 0) {
        print_divider("模拟文件系统");
        fs_pwd();
        printf("1. 创建目录 mkdir\n");
        printf("2. 创建文件 create\n");
        printf("3. 写文件 write\n");
        printf("4. 读文件 read\n");
        printf("5. 删除 delete\n");
        printf("6. 列目录 ls\n");
        printf("7. 切换目录 cd\n");
        printf("8. 查看空闲块位图\n");
        printf("0. 返回上一级\n");
        choice = read_int("请选择操作: ");

        switch (choice) {
            case 1:
                fs_mkdir();
                break;
            case 2:
                fs_create_file();
                break;
            case 3:
                fs_write_file();
                break;
            case 4:
                fs_read_file();
                break;
            case 5:
                fs_delete();
                break;
            case 6:
                fs_list();
                break;
            case 7:
                fs_cd();
                break;
            case 8:
                print_bitmap();
                break;
            case 0:
                break;
            default:
                printf("无效选项。\n");
                break;
        }
    }
}
