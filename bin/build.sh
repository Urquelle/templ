pushd ${dev_dir}/Cpp/build
clang -g -ferror-limit=2 -Wno-logical-op-parentheses -I../templ/src ../templ/examples/main.cpp -lm -otempl
popd
