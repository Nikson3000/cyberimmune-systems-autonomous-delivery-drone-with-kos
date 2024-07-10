#! /usr/bin/bash
rm -f mavproxy/mav.parm
rm -f mavproxy/mav.tlog
rm -f mavproxy/mav.tlog.raw
rm -f ardupilot/eeprom.bin
rm -rf ardupilot/logs
rm -rf ardupilot/terrain
cd planner
xterm  -geometry 100x30 -bg '#300924' -fg white -e './APM_Planner.AppImage' & disown
cd ../kos
if [[ $* == *"--no-server"* ]]
	then
		xterm -geometry 100x30 -bg '#300924' -fg white -e './cross-build-sim-offline.sh' & disown
	else
		xterm -geometry 100x30 -bg '#300924' -fg white -e './cross-build-sim-online.sh' & disown
fi
cd ../ardupilot
if [[ $* == *"--with-obstacles"* ]]
	then
		gnome-terminal -- ./run_in_terminal_window.sh ArduCopter sitl_obstacles/bin/arducopter -S --model + --speedup 1 --slave 0 --serial5=tcp:5765:wait --serial6=tcp:5766:wait --serial7=tcp:5767:wait --defaults copter.parm --sim-address=127.0.0.1 --home=46.6143745,142.8119421,69.45,0 -I0
	else
		gnome-terminal -- ./run_in_terminal_window.sh ArduCopter sitl/bin/arducopter -S --model + --speedup 1 --slave 0 --serial5=tcp:5765:wait --serial6=tcp:5766:wait --serial7=tcp:5767:wait --defaults copter.parm --sim-address=127.0.0.1 --home=46.6143745,142.8119421,69.45,0 -I0
fi
cd ../mavproxy
python3 MAVProxy/mavproxy.py --out 127.0.0.1:14550 --out 127.0.0.1:14551 --master tcp:127.0.0.1:5760 --sitl 127.0.0.1:5501

# 46.6143886,142.8119468,69.20
# 53.1019446,107.3774394,846.22
