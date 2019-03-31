struct Frame {
    char *mem;
    size_t used;
    size_t size;
};

internal_proc Frame *
frame_new(size_t size) {
    Frame *result = (Frame *)xmalloc(sizeof(Frame));

    result->mem  = (char *)xmalloc(size);
    result->used = 0;
    result->size = size;

    return result;
}

internal_proc size_t
frame_set(Frame *frame, void *val, size_t size) {
    if ( frame->used + size > frame->size ) {
        assert(!"framespeicher überlauf");
    }

    size_t result = frame->used;
    memcpy((char *)frame->mem + frame->used, val, size);
    frame->used += size;

    return result;
}
