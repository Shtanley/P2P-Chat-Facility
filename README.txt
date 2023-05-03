Errors:
	1. "incomplete type error on struct addrinfo"		(FIXED) 
	2. undefined references to "addrinfo", "AI_PASSIVE"	(FIXED)
	3. "asm/socket.h: No such file or directory"		(FIXED)
	4. segmentation fault on pressing "!" to terminate remote connection to peer	(IN PROGRESS)
	5. sender would recieve the message that was being sent	(FIXED)
	6. "tcache_thread_shutdown(): unaligned tcache chunk detected"	(FIXED)

Debug log:
	1, 2 & 3: added _GNU_SOURCE to c_cpp_properties.json, 
	   "defines": ["__linux__", "__x86_64__", "_GNU_SOURCE"], 
	   which I found here: https://github.com/Microsoft/vscode-cpptools/issues/2025
	   
	6: added helper functions to list.c to help debug list.c
	   to fix the issue were when sending a messegae you would instead recieve it.

	5: moved UDPsockets functions to main, however I moved it back as it makes it 
	   easier to debug error#4.

	6: tried a different implementation for UDPthreads & list functions but 
	   the same error occurs, old implementation is commented out.
	   Added helper functions to list and main to organize code for 
	   debugging memory leaks.

References:
	1. Beej's Guide to Network Programming. (n.d.). Retrieved February 25, 2023, 
   	   from https://beej.us/guide/bgnet/html/#client-server-background 

	2. Troan, E., &amp; By. (2021, June 22). Multiplexed I/0 with poll() tutorial. 
   	   Linux Today. Retrieved February 22, 2023, 
   	   from https://www.linuxtoday.com/blog/multiplexed-i0-with-poll/ 