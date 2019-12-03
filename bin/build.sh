pushd ${dev_dir}/Cpp/build
clang -g -Wno-logical-op-parentheses -I../templ/src ../templ/src/main.cpp -lm -otempl
popd
