#!/bin/bash

./compile_mlcommon.sh $1
	EXIT_CODE=$?
	if [[ $EXIT_CODE -ne 0 ]]; then
		echo -e "\n\tcompilation failed --> mlcommon"
		break
	fi

ls compile_[^all]*.sh | while read -r line; do
	./$line $1
	EXIT_CODE=$?
	if [[ $EXIT_CODE -ne 0 ]]; then
		echo -e "\n\tcompilation failed --> $line"
		break
	fi
done
