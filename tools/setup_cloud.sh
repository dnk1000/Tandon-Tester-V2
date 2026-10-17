#!/usr/bin/env bash
set -euo pipefail
repo=/workspace/Tandon-Tester-V2
sdk=/workspace/esp-idf
export IDF_TOOLS_PATH=/workspace/.espressif
export IDF_PYTHON_CHECK_CONSTRAINTS=no
if [[ ! -d "$sdk" ]]; then
    git clone --depth 1 --branch v5.4.3 --recursive --shallow-submodules \
        https://github.com/espressif/esp-idf.git "$sdk"
fi
# Never reset or replace an existing checkout.
[[ "$(git -C "$sdk" rev-parse HEAD)" == ea1c174c1cbb7348bd8ba0ff1eb306246938dd80 ]]
python3 -m venv /workspace/tm100-tools
/workspace/tm100-tools/bin/python -m pip install cmake==4.4.4 ninja==1.13.2
python3 "$sdk/tools/idf_tools.py" --non-interactive install --targets=esp32s3
# The extra Espressif wheels/constraints host may be restricted in cloud tasks.
# Use the supported PyPI-only mode, then the exact Python versions used for validation.
python3 "$sdk/tools/idf_tools.py" install-python-env --no-constraints --extra-wheels-url ''
python_minor=$(python3 -c 'import sys; print(f"{sys.version_info.major}.{sys.version_info.minor}")')
"$IDF_TOOLS_PATH/python_env/idf5.4_py${python_minor}_env/bin/python" -m pip install \
    -r "$repo/tools/cloud-python-requirements.txt"
. "$repo/tools/activate_cloud.sh"
cd "$repo"
idf.py -DIDF_TARGET=esp32s3 build
cmake -S tests -B tests/build -G Ninja
cmake --build tests/build
ctest --test-dir tests/build --output-on-failure
