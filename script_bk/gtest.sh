#! /bin/bash

rm $(find ./ -name "*.gcda")
rm coverage.info
rm coverage_out -rf

./mk.sh
./app/bin/monitor_gtest


lcov -c -d ./build_/monitor/  -o  coverage.info  \
	--include '*/src/*'  \
	--exclude "*/src/json.hpp" --exclude "*/src/communication/dds_helper*"

genhtml coverage.info --output-directory coverage_out