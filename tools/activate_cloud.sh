# @file activate_cloud.sh
# @brief Source-only activation of the prepared cloud toolchain.
#
# Source this file: . tools/activate_cloud.sh
export IDF_TOOLS_PATH=/workspace/.espressif
# Cloud installation uses the tested, pinned Python snapshot in tools/.
# This skips optional version constraints, NOT TLS, signatures or tool checksums.
export IDF_PYTHON_CHECK_CONSTRAINTS=no
export PATH=/workspace/tm100-tools/bin:$PATH
. /workspace/esp-idf/export.sh
