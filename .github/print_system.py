# flake8: noqa
### USAGE:
#           conda activate ve-AICO
#           python print_system.py
### Terminal log:
#           Platform:   Linux-5.11.0-36-generic-x86_64-with-glibc2.17
#           PyTorch:    1.9.1
#           NumPy:      1.20.3
#           Python:     3.8.11 (default, Aug  3 2021, 15:09:35)
#           [GCC 7.5.0]

import sys
import platform
import torch
import numpy

print('Platform:  ', platform.platform())
print('PyTorch:   ', torch.__version__)
print('NumPy:     ', numpy.__version__)
print('Python:    ', sys.version)
