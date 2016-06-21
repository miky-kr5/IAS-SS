# Copyright (c) 2016, Miguel Angel Astor Romero
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
