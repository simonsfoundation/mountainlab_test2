#!/bin/bash

ls compile_[^all]*.sh | while read -r line; do
	./$line
done
