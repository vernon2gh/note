#ifndef __DEBUG__
#define __DEBUG__

#define ESC_START     "\033["
#define ESC_END       "\033[0m"
#define COLOR_FATAL   "31;40;5m"
#define COLOR_ALERT   "31;40;1m"
#define COLOR_CRIT    "31;40;1m"
#define COLOR_ERROR   "31;40;1m"
#define COLOR_WARN    "33;40;1m"
#define COLOR_NOTICE  "34;40;1m"
#define COLOR_INFO    "32;40;1m"
#define COLOR_DEBUG   "36;40;1m"
#define COLOR_TRACE   "37;40;1m"

#define PRI_INFO(format, args...) \
	(printf( ESC_START COLOR_INFO "[INFO] %s: %s: %d: " format ESC_END, __FILE__, __FUNCTION__ , __LINE__, ##args))

#ifdef DEBUG
	#define PRI_DEBUG(format, args...) \
		(printf( ESC_START COLOR_DEBUG "[DEBUG] %s: %s: %d: " format ESC_END, __FILE__, __FUNCTION__ , __LINE__, ##args))
#else
	#define PRI_DEBUG(format, args...)
#endif

#define PRI_WARN(format, args...) \
	(printf( ESC_START COLOR_WARN "[WARN] %s: %s: %d: " format ESC_END, __FILE__, __FUNCTION__ , __LINE__, ##args))
#define PRI_ERROR(format, args...) \
	(printf( ESC_START COLOR_ERROR "[ERROR] %s: %s: %d: " format ESC_END, __FILE__, __FUNCTION__ , __LINE__, ##args))

#endif

