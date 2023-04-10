all:
	@mkdir -p bin

	make -C create_mbr
	mv create_mbr/update_mbr bin/

	make -C parser_mbr
	mv parser_mbr/parser_mbr bin/

	make -C script
	mv script/script bin/

	make -C sunxi-mbr
	mv sunxi-mbr/sunxi-mbr bin/

	cp u_boot_env_gen bin/
	cp dragonsecboot bin/

clean:
	make -C create_mbr clean
	make -C parser_mbr clean
	make -C script clean
	make -C sunxi-mbr clean
	rm -rf bin