import os
import re
import tarfile
import urllib.request
import subprocess
import time
import matplotlib.pyplot as plt
import numpy as np
from collections import defaultdict
import tempfile
import shutil

# URLs for SAT problems
URLS = (
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uf20-91.tar.gz",
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uf50-218.tar.gz",
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uuf50-218.tar.gz",
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uf75-325.tar.gz",
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uuf75-325.tar.gz",
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uf100-430.tar.gz",
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uuf100-430.tar.gz",
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uf125-538.tar.gz",
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uf150-645.tar.gz",
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uuf150-645.tar.gz",
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uf175-753.tar.gz",
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uuf175-753.tar.gz",
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uf200-860.tar.gz",
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uuf200-860.tar.gz",
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uf225-960.tar.gz",
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uuf225-960.tar.gz",
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uf250-1065.tar.gz",
    "https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/RND3SAT/uuf250-1065.tar.gz",
)


class SATBenchmark:
    def __init__(self, solver_path="./dpll", num_runs=3, temp_dir=None):
        """
        Initialize the SAT benchmark

        Args:
            solver_path (str): Path to the dpll solver executable
            num_runs (int): Number of times to run each test for averaging
            temp_dir (str): Temporary directory for downloaded files
        """
        self.solver_path = solver_path
        self.num_runs = num_runs
        self.temp_dir = temp_dir or tempfile.mkdtemp(prefix="sat_benchmark_")
        self.results = defaultdict(list)

        # Verify solver exists
        if not os.path.exists(self.solver_path):
            raise FileNotFoundError(f"SAT solver not found at {self.solver_path}")

        print(f"Using temporary directory: {self.temp_dir}")

    def download_and_extract(self, url):
        """Download and extract a tar.gz file"""
        filename = os.path.basename(url)
        local_path = os.path.join(self.temp_dir, filename)

        print(f"Downloading {filename}...")
        urllib.request.urlretrieve(url, local_path)

        print(f"Extracting {filename}...")
        with tarfile.open(local_path, "r:gz") as tar:
            tar.extractall(self.temp_dir)

        # Remove the archive after extraction
        os.remove(local_path)

    def get_expected_result(self, filename):
        """
        Determine expected result based on filename
        uf = SAT, uuf = UNSAT
        """
        basename = os.path.basename(filename)
        if basename.startswith("uf") and not basename.startswith("uuf"):
            return "SAT"
        elif basename.startswith("uuf"):
            return "UNSAT"
        else:
            raise ValueError(f"Cannot determine expected result for {filename}")

    def get_variable_count(self, filename):
        """Extract variable count from filename"""
        basename = os.path.basename(filename)
        # Extract the number after 'uf' or 'uuf'
        if basename.startswith("uf") and not basename.startswith("uuf"):
            # uf20-0163.cnf -> 20
            return int(basename[2:].split("-")[0])
        elif basename.startswith("uuf"):
            # uuf50-001.cnf -> 50
            return int(basename[3:].split("-")[0])
        else:
            raise ValueError(f"Cannot extract variable count from {filename}")

    def run_solver(self, cnf_file):
        """Run the SAT solver on a CNF file and return result and execution time"""
        start_time = time.time()
        try:
            result = subprocess.run(
                [self.solver_path, cnf_file],
                capture_output=True,
                text=True,
                timeout=3600,  # 5 minute timeout
            )
            execution_time = time.time() - start_time

            if result.returncode != 0:
                raise RuntimeError(
                    f"Solver failed with return code {result.returncode}"
                )

            # Extract first line (SAT/UNSAT)
            output_lines = result.stdout.strip().split("\n")
            if not output_lines:
                raise ValueError("No output from solver")

            sat_result = output_lines[0].strip()
            return sat_result, execution_time

        except subprocess.TimeoutExpired:
            execution_time = time.time() - start_time
            raise TimeoutError(f"Solver timed out after {execution_time:.2f} seconds")

    def test_file(self, cnf_file):
        """Test a single CNF file multiple times and return average results"""
        expected = self.get_expected_result(cnf_file)
        variable_count = self.get_variable_count(cnf_file)

        print(
            f"Testing {os.path.basename(cnf_file)} (expected: {expected}, variables: {variable_count})"
        )

        times = []
        actual_results = []

        for run in range(self.num_runs):
            try:
                result, exec_time = self.run_solver(cnf_file)
                times.append(exec_time)
                actual_results.append(result)

                # Verify result matches expectation
                if result != expected:
                    raise ValueError(
                        f"Result mismatch for {cnf_file}: expected {expected}, got {result}"
                    )

                print(f"  Run {run + 1}/{self.num_runs}: {result} in {exec_time:.4f}s")

            except Exception as e:
                print(f"  Run {run + 1}/{self.num_runs}: ERROR - {e}")
                raise

        # Verify all runs produced consistent results
        if len(set(actual_results)) != 1:
            raise ValueError(f"Inconsistent results across runs: {set(actual_results)}")

        avg_time = np.mean(times)
        std_time = np.std(times)

        return {
            "file": cnf_file,
            "expected": expected,
            "variable_count": variable_count,
            "avg_time": avg_time,
            "std_time": std_time,
            "all_times": times,
        }

    def find_cnf_files(self):
        """Find all .cnf files in the temporary directory"""
        cnf_files = []
        for root, dirs, files in os.walk(self.temp_dir):
            for file in files:
                if file.endswith(".cnf"):
                    cnf_files.append(os.path.join(root, file))

        # After all we need to return them in variables ascending order.
        # If we will just sort them, it will not work.
        cnf_files = sorted(
            cnf_files,
            key=lambda fname: (
                int(match.group(2)),
                int(match.group(3))
            ) if (match := re.match(r'(uf|uff)(\d+)-(\d+).cnf', os.path.basename(fname)))
            else (float('inf'), float('inf'))
        )
        return cnf_files

    def run_benchmark(self):
        """Run the complete benchmark"""
        print("Starting SAT benchmark...")

        # Download and extract all archives
        for url in URLS:
            self.download_and_extract(url)

        # Find all CNF files
        cnf_files = self.find_cnf_files()
        print(f"Found {len(cnf_files)} CNF files")

        if not cnf_files:
            raise ValueError("No CNF files found after extraction")

        # Test each file
        successful_tests = 0
        for cnf_file in cnf_files:
            try:
                result = self.test_file(cnf_file)
                self.results[result["variable_count"]].append(result)
                successful_tests += 1
                print(
                    f"✓ {os.path.basename(cnf_file)}: {result['avg_time']:.4f}s ± {result['std_time']:.4f}s\n"
                )

            except Exception as e:
                print(f"✗ {os.path.basename(cnf_file)}: FAILED - {e}\n")

        print(
            f"Benchmark completed: {successful_tests}/{len(cnf_files)} files successful"
        )

    def plot_results(self):
        """Plot the benchmark results using matplotlib"""
        if not self.results:
            print("No results to plot")
            return

        # Prepare data for plotting
        sat_data = defaultdict(list)
        unsat_data = defaultdict(list)

        for var_count, file_results in self.results.items():
            for result in file_results:
                if result["expected"] == "SAT":
                    sat_data[var_count].append(result["avg_time"])
                else:
                    unsat_data[var_count].append(result["avg_time"])

        # Calculate averages for each variable count
        sat_vars = sorted(sat_data.keys())
        unsat_vars = sorted(unsat_data.keys())

        sat_avg_times = [np.mean(sat_data[var]) for var in sat_vars]
        sat_std_times = [np.std(sat_data[var]) for var in sat_vars]

        unsat_avg_times = [np.mean(unsat_data[var]) for var in unsat_vars]
        unsat_std_times = [np.std(unsat_data[var]) for var in unsat_vars]

        # Create the plot
        plt.figure(figsize=(12, 8))

        # Plot SAT results
        if sat_vars:
            plt.errorbar(
                sat_vars,
                sat_avg_times,
                yerr=sat_std_times,
                fmt="o-",
                label="SAT",
                capsize=5,
                linewidth=2,
                markersize=6,
            )

        # Plot UNSAT results
        if unsat_vars:
            plt.errorbar(
                unsat_vars,
                unsat_avg_times,
                yerr=unsat_std_times,
                fmt="s-",
                label="UNSAT",
                capsize=5,
                linewidth=2,
                markersize=6,
            )

        plt.xlabel("Number of Variables")
        plt.ylabel("Average Execution Time (seconds)")
        plt.title("SAT Solver Performance Benchmark")
        plt.legend()
        plt.grid(True, alpha=0.3)
        plt.yscale("log")  # Use log scale for better visualization of wide time ranges

        # Add some statistics to the plot
        total_files = sum(len(results) for results in self.results.values())
        plt.figtext(
            0.02,
            0.02,
            f"Total files tested: {total_files}\nRuns per file: {self.num_runs}",
            fontsize=10,
            bbox=dict(boxstyle="round,pad=0.3", facecolor="lightgray"),
        )

        plt.tight_layout()
        plt.savefig("sat_benchmark_results.png", dpi=300, bbox_inches="tight")
        plt.show()

        print(f"Plot saved as 'sat_benchmark_results.png'")

    def print_summary(self):
        """Print a summary of the benchmark results"""
        if not self.results:
            print("No results to summarize")
            return

        print("\n" + "=" * 60)
        print("BENCHMARK SUMMARY")
        print("=" * 60)

        total_files = sum(len(results) for results in self.results.values())
        total_sat = 0
        total_unsat = 0

        for var_count, file_results in self.results.items():
            sat_count = sum(1 for r in file_results if r["expected"] == "SAT")
            unsat_count = len(file_results) - sat_count

            total_sat += sat_count
            total_unsat += unsat_count

            avg_time_sat = (
                np.mean([r["avg_time"] for r in file_results if r["expected"] == "SAT"])
                if sat_count > 0
                else 0
            )
            avg_time_unsat = (
                np.mean(
                    [r["avg_time"] for r in file_results if r["expected"] == "UNSAT"]
                )
                if unsat_count > 0
                else 0
            )

            print(
                f"Variables: {var_count:3d} | Files: {len(file_results):3d} | "
                f"SAT: {sat_count:2d} | UNSAT: {unsat_count:2d} | "
                f"Avg Time SAT: {avg_time_sat:.4f}s | Avg Time UNSAT: {avg_time_unsat:.4f}s"
            )

        print("=" * 60)
        print(f"TOTAL: {total_files} files ({total_sat} SAT, {total_unsat} UNSAT)")
        print(f"Configuration: {self.num_runs} runs per file")

    def cleanup(self):
        """Clean up temporary files"""
        if os.path.exists(self.temp_dir):
            shutil.rmtree(self.temp_dir)
            print(f"Cleaned up temporary directory: {self.temp_dir}")


def main():
    # Configuration
    SOLVER_PATH = "./dpll"  # Path to your dpll executable
    NUM_RUNS = 3  # Number of runs per file for averaging

    try:
        # Initialize benchmark
        benchmark = SATBenchmark(solver_path=SOLVER_PATH, num_runs=NUM_RUNS)

        try:
            # Run the benchmark
            benchmark.run_benchmark()

            # Print summary
            benchmark.print_summary()

            # Plot results
            benchmark.plot_results()

        finally:
            # Cleanup temporary files
            benchmark.cleanup()

    except Exception as e:
        print(f"Benchmark failed: {e}")
        return 1

    return 0


if __name__ == "__main__":
    exit(main())
