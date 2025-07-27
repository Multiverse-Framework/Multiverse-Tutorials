import math

# <!-- Extreme amgles -->
# <!-- <body name="laser_mount" pos="0 0 1">
#   <site name="laser_site_0" pos="0.202 0 0.0945" euler="1.57 0 0" />
#   <site name="laser_site_1" pos="0.202 0 0.0945" euler="-1.57 0 0" />
# </body> -->

num_beams = 721
start_angle = 0  # degrees
angle_increment = 180 / (num_beams - 1)

print('<body name="laser_mount" pos="0 0 1">')
for i in range(num_beams):
    angle_deg = (start_angle/2) + i * angle_increment
    if angle_deg < 90:
        angle_rad = math.radians(angle_deg)
        print(f'  <site name="laser_site_{i}" pos="0.202 0 0.0945" euler="1.5708 {angle_rad:.6f}  0" />')
    else:
        angle_rad = math.radians(180 - angle_deg)
        print(f'  <site name="laser_site_{i}" pos="0.202 0 0.0945" euler="-1.5708 {angle_rad:.6f}  0" />')

print('</body>\n')

# print('<sensor>')
# for i in range(num_beams):
#     print(f'  <rangefinder name="laser_{i}" site="laser_site_{i}" />')
# print('</sensor>')

