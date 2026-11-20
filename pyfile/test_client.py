import socket
import json
import time
import math

UDP_IP = "127.0.0.1"
UDP_PORT = 12345

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def send_data(data):
    json_str = json.dumps(data, ensure_ascii=False)
    sock.sendto(json_str.encode('utf-8'), (UDP_IP, UDP_PORT))
    print(f"Sent: {json_str}")

print("Sending initial data...")

own_ship = {
    "type": "ownShip",
    "mmsi": "123456789",
    "name": "OwnShip",
    "lon": 121.5,
    "lat": 31.2,
    "heading": 45.0,
    "speed": 12.5,
    "validDuration": 10000
}
send_data(own_ship)

time.sleep(0.1)

target1 = {
    "type": "aisTarget",
    "mmsi": "987654321",
    "name": "MerchantA",
    "lon": 121.51,
    "lat": 31.22,
    "heading": 180.0,
    "speed": 8.0,
    "shipType": 5,
    "isDanger": False,
    "validDuration": 10000
}
send_data(target1)

time.sleep(0.1)

target2 = {
    "type": "aisTarget",
    "mmsi": "112233445",
    "name": "FishingB",
    "lon": 121.48,
    "lat": 31.18,
    "heading": 90.0,
    "speed": 5.0,
    "shipType": 30,
    "isDanger": False,
    "validDuration": 10000
}
send_data(target2)

time.sleep(0.1)

target3 = {
    "type": "aisTarget",
    "mmsi": "556677889",
    "name": "DangerTarget",
    "lon": 121.55,
    "lat": 31.25,
    "heading": 225.0,
    "speed": 15.0,
    "shipType": 50,
    "isDanger": True,
    "validDuration": 10000
}
send_data(target3)

time.sleep(0.1)

radar = {
    "type": "sensor",
    "id": "radar_01",
    "lon": 121.5,
    "lat": 31.2,
    "radius": 10000,
    "sensorType": "radar",
    "active": True,
    "validDuration": 10000
}
send_data(radar)

time.sleep(0.1)

sonar = {
    "type": "sensor",
    "id": "sonar_01",
    "lon": 121.5,
    "lat": 31.2,
    "radius": 3000,
    "sensorType": "sonar",
    "active": True,
    "validDuration": 10000
}
send_data(sonar)

time.sleep(0.1)

weapon = {
    "type": "weapon",
    "id": "missile_01",
    "lon": 121.5,
    "lat": 31.2,
    "targetLon": 121.55,
    "targetLat": 31.25,
    "weaponType": "missile",
    "active": True,
    "validDuration": 10000
}
send_data(weapon)

time.sleep(0.1)

marker1 = {
    "type": "marker",
    "id": "mark_01",
    "lon": 121.52,
    "lat": 31.23,
    "label": "航道标记",
    "color": "#FFD700",
    "validDuration": 10000
}
send_data(marker1)

print("\nInitial data sent. Starting dynamic updates...")

own_lon = 121.5
own_lat = 31.2
own_heading = 45.0

target1_lon = 121.51
target1_lat = 31.22
target1_heading = 180.0

target2_lon = 121.48
target2_lat = 31.18
target2_heading = 90.0

target3_lon = 121.55
target3_lat = 31.25
target3_heading = 225.0

try:
    while True:
        own_lon += 0.0001
        own_lat += 0.00005
        own_heading = (own_heading + 0.5) % 360

        own_ship["lon"] = own_lon
        own_ship["lat"] = own_lat
        own_ship["heading"] = own_heading
        send_data(own_ship)

        angle1 = 60.0 * math.pi / 180.0
        target1_lon += 0.00005 * math.cos(angle1)
        target1_lat += 0.00005 * math.sin(angle1)
        target1_heading = (target1_heading + 1.0) % 360

        target1["lon"] = target1_lon
        target1["lat"] = target1_lat
        target1["heading"] = target1_heading
        send_data(target1)

        angle2 = 120.0 * math.pi / 180.0
        target2_lon += 0.00005 * math.cos(angle2)
        target2_lat += 0.00005 * math.sin(angle2)
        target2_heading = (target2_heading + 1.0) % 360

        target2["lon"] = target2_lon
        target2["lat"] = target2_lat
        target2["heading"] = target2_heading
        send_data(target2)

        angle3 = 180.0 * math.pi / 180.0
        target3_lon += 0.00005 * math.cos(angle3)
        target3_lat += 0.00005 * math.sin(angle3)
        target3_heading = (target3_heading + 1.0) % 360

        target3["lon"] = target3_lon
        target3["lat"] = target3_lat
        target3["heading"] = target3_heading
        send_data(target3)

        radar["lon"] = own_lon
        radar["lat"] = own_lat
        send_data(radar)

        sonar["lon"] = own_lon
        sonar["lat"] = own_lat
        send_data(sonar)

        weapon["lon"] = own_lon
        weapon["lat"] = own_lat
        weapon["targetLon"] = target3_lon
        weapon["targetLat"] = target3_lat
        send_data(weapon)

        time.sleep(1)

except KeyboardInterrupt:
    print("\nStopping...")
    sock.close()