struct Frame {
    void *mem;
};

internal_proc Frame *
frame_new(size_t size) {
    Frame *result = (Frame *)xmalloc(sizeof(Frame));

    result->mem = xmalloc(size);

    return result;
}
