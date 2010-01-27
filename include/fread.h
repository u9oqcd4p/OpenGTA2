#ifndef __HEADER_FREAD
#define __HEADER_FREAD


/**
 * Macro which checks the return value of fread. It will log a message with the
 * user defined logging function ,,log_func'' if an error occured.
 * 
 * @param dest void* restrict, first argument of fread
 * @param size size_t, second argument of fread
 * @param times size_t, third argument of fread
 * @param src FILE* restrict, forth argument of fread
 * @param filename char const* which contains the filename (or any form of
 *     description where src comes from, only used for debugging purposes)
 * @param log_func function with with an error is to log
 *
 * @see fread
 */
#define FREAD(dest, size, times, src, filename, log_func)		\
{									\
	size_t const bytes_expected = (size) * (times);			\
	size_t const bytes_read = fread(				\
		(dest), (size), (times), (src));			\
									\
	if (bytes_expected != bytes_read) {				\
		log_func("Expected to read %d (%d * %d) bytes from %s into %s but could only read %d bytes in %s:%d (%s)",									\
			bytes_expected, (size), (times),		\
			filename, #dest,				\
			bytes_read,					\
			__FILE__, __LINE__, __func__			\
		);							\
	}								\
}									\



#endif

