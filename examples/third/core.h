// Export macro definition for cross-platform compatibility.

// Export from plugin
#if defined(WIN)
	#if defined(PLUGIN)
		#define PLUGIN_API __declspec(dllexport)
	#else
		#define PLUGIN_API __declspec(dllimport)
	#endif
#elif defined(MAC) || defined(LIN)
	#if defined(PLUGIN)
		#if defined(__GNUC__) || defined(__clang__)
			#define PLUGIN_API __attribute__((visibility("default")))
		#else
			#define PLUGIN_API
		#endif
	#else
		#define PLUGIN_API
	#endif
#else
	// Fallback: assume Unix-like visibility attributes
	#if defined(PLUGIN)
		#if defined(__GNUC__) || defined(__clang__)
			#define PLUGIN_API __attribute__((visibility("default")))
		#else
			#define PLUGIN_API
		#endif
	#else
		#define PLUGIN_API
	#endif
#endif