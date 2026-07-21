#ifndef __ID_H__
#define __ID_H__

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

// OID may substitute with abstract base class of object id in the future.
// And we do not want to expose OID details to the user, so OID* is used here.
class OID;

// A wrapper of object id object.
class ENCL_API EnclObjectPtr 
{
public:
	EnclObjectPtr();
	EnclObjectPtr(OID * pid);

	~EnclObjectPtr();

	// implicit conversion
	operator OID*();
	operator OID*()const;

	bool operator == (OID* pid);
	bool operator == (OID* pid) const;

	bool operator == (EnclObjectPtr otherId);
	bool operator == (EnclObjectPtr otherId) const;

	bool operator != (OID* pid);
	bool operator != (OID* pid) const;

	EnclObjectPtr & operator = (EnclObjectPtr other);
	EnclObjectPtr & operator = (OID* pid);

	bool IsNull()const;

	friend void swap_(EnclObjectPtr & first, EnclObjectPtr & second);
	EnclObjectPtr(const EnclObjectPtr & other);
private:
	OID * m_id;
};

#endif