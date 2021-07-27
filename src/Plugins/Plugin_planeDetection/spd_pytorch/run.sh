#!/bin/bash
GPU_ID=$2
filename=$1

CUDA_VISIBLE_DEVICES=""
CUDA_VISIBLE_DEVICES=${GPU_ID}
export CUDA_VISIBLE_DEVICES

python $filename
