from pathlib import Path
import os
import sys

ROOT = Path(__file__).resolve().parent.parent
MPL_CACHE = ROOT / ".mplcache"
MPL_CACHE.mkdir(parents=True, exist_ok=True)
os.environ.setdefault("MPLCONFIGDIR", str(MPL_CACHE))

try:
    import pandas as pd
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
except ImportError as exc:
    print(f"Missing dependency: {exc}")
    sys.exit(1)

RESULTS = ROOT / "results" / "summary.csv"
FIGURES = ROOT / "figures"


def save_metric_chart(df: pd.DataFrame, metric: str, filename: str) -> None:
    pivot = df.pivot(index="scenario", columns="algorithm", values=metric)
    ax = pivot.plot(kind="bar", figsize=(10, 6))
    ax.set_title(metric)
    ax.set_xlabel("scenario")
    ax.set_ylabel(metric)
    ax.legend(title="algorithm")
    plt.tight_layout()
    plt.savefig(FIGURES / filename, dpi=150)
    plt.close()


def main() -> int:
    if not RESULTS.exists():
        print(f"summary.csv not found: {RESULTS}")
        return 1

    FIGURES.mkdir(parents=True, exist_ok=True)
    df = pd.read_csv(RESULTS)

    save_metric_chart(df, "average_turnaround_time", "avg_turnaround_time.png")
    save_metric_chart(df, "average_waiting_time", "avg_waiting_time.png")
    save_metric_chart(df, "average_response_time", "avg_response_time.png")
    save_metric_chart(df, "context_switch_count", "context_switch_count.png")
    save_metric_chart(df, "starvation_rate", "starvation_rate.png")
    save_metric_chart(df, "fairness_index", "fairness_index.png")
    print("Figures generated in figures/")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
