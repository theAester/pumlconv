conv: conv.c
	@gcc conv.c -o conv

install:
	@mv conv /usr/bin/
	@cp ac.sh /usr/bin/

remove:
	@rm /usr/bin/conv
	@rm /usr/bin/ac.sh
