#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#if defined __MSW__ 
	#ifdef CMC_ECDISSDK_LIB
		#define ENCL_API __declspec(dllexport) 
	#else
		#define ENCL_API __declspec(dllimport) 
	#endif
#else
	#ifdef __LINUX__
		#ifdef CMC_ECDISSDK_LIB
			#define ENCL_API __attribute__ ((visibility ("default"))) 
		#else
			#define ENCL_API
		#endif
	#else
        #define ENCL_API
	#endif
#endif

#endif
