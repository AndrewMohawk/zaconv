#!/bin/bash
badgeNumber=1338
for i in {1..2}
do
	let badgeNumber++
	echo "Build firmware for badge $badgeNumber"
	sed -i '' --expression="s/1338;/$badgeNumber;/" src/badge.ino
	ino build
	cp /home/rc1140/dev/zaconv/badgeCode/.build/uno/firmware.hex firmware$i.hex
	git reset --hard
done

