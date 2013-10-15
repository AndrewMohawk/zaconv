#!/bin/bash
for i in {1..2}
do
	sed -i '' --expression="s/1338;/133$i;/" src/badge.ino
	ino build
	cp /home/rc1140/dev/zaconv/badgeCode/.build/uno/firmware.hex firmware$i.hex
	git reset --hard
done

