pushd ${dev_dir}/Cpp/build
clang -g -Wno-logical-op-parentheses -I../templ/src ../templ/examples/main.cpp -lm -otempl
popd
