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
)
