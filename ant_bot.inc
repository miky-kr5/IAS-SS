define ant_sensor sensor
(
	size [0.01 0.05 0.01 ] 
	range [0 5.0]
	fov 60
	samples 1
	color_rgba [ 0 1 0 0.2 ] 
)

define ant_sonars ranger
(
	ant_sensor( pose [ 0 0 0 -60 ] )
	ant_sensor( pose [ 0 0 0   0 ] )
	ant_sensor( pose [ 0 0 0  60 ] )
)

define ant_bot position
(
	block
	(
		points 6
		point[0] [ 0.75 0 ]
		point[1] [ 1 0.25 ]
		point[2] [ 1 0.75 ]
		point[3] [ 0.75 1 ]
		point[4] [ 0 1 ]
		point[5] [ 0 0 ]
		z [ 0 0.45 ]
	)

	drive "diff"
	localization "gps"

	ant_sonars()
)
