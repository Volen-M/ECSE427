sma_free : This function will break the memory allocation is invoked in the middle of another block. As the size of the block it is in the middle of will not be updated. This in turn will cause a double allocation of said memory that will be "freed".

All internally passed pointers are header pointers. get_blockSize() was as such changed to fit the standard. 

The SMA Statistics tests only show the space not covered by headers. As such there is a descrepency between free space and actual free space one is able to allocate given that Free Headers are smaller than Allocated Headers