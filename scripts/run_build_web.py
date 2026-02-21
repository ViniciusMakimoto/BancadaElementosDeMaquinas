"""
This script is executed by PlatformIO before the build process.
It locates the Python executable within the project's virtual environment (.venv)
and uses it to run the 'embed_html.py' script, ensuring that the correct
dependencies are available.
"""
import subprocess
import sys
import os

# Get the project directory
project_dir = os.getcwd()

# Determine the path to the venv Python executable
if sys.platform == "win32":
    python_executable = os.path.join(project_dir, ".venv", "Scripts", "python.exe")
else:
    python_executable = os.path.join(project_dir, ".venv", "bin", "python")

script_to_run = os.path.join(project_dir, "scripts", "build.py")

print("--- Running pre-build script: build.py ---")

# Check if the venv Python executable exists
if not os.path.exists(python_executable):
    print(f"Error: Python executable not found at {python_executable}")
    sys.exit(1)

# Run the script using the venv's Python
try:
    result = subprocess.run(
        [python_executable, script_to_run],
        capture_output=True,
        text=True,
        check=True,
        cwd=project_dir
    )
    if result.stdout:
        print(result.stdout)
    if result.stderr:
        print("Error output:")
        print(result.stderr)
    print("--- Pre-build script finished successfully ---")
except subprocess.CalledProcessError as e:
    print("!!! Error executing pre-build script !!!")
    if e.stdout:
        print(e.stdout)
    if e.stderr:
        print(e.stderr)
    sys.exit(1)
except FileNotFoundError:
    print(f"Error: The script '{script_to_run}' was not found.")
    sys.exit(1)

