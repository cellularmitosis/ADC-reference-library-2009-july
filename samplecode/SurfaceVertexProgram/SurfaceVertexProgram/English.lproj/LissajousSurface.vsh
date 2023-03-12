!!ARBvp1.0

ATTRIB 	vPos  	= vertex.position;

# Constants
PARAM 	negOne		= {-1.000000000000000, -1.000000000000000, -1.000000000000000, -1.000000000000000};
PARAM 	one6		= {-0.166666666666767, -0.166666666666667, -0.166666666666667, -0.166666666666667};
PARAM 	one120		= { 8.333333333330e-3,  8.333333333333e-3,  8.333333333330e-3,  8.333333333333e-3};
PARAM 	one5040 	= {-1.984126984130e-4, -1.984126984130e-4, -1.984126984130e-4, -1.984126984130e-4};
PARAM 	one362880 	= { 2.755731922400e-6,  2.755731922400e-6,  2.755731922400e-6,  2.755731922400e-6};
PARAM 	one39916800 	= {-2.505210838000e-8, -2.505210838000e-8, -2.505210838000e-8, -2.505210838000e-8};
PARAM 	oneHoleLot 	= { 1.60590438368e-10,  1.60590438368e-10,  1.60590438368e-10,  1.60590438368e-10};

# Tempory variables
TEMP 	tPos;
TEMP	temp0, temp1, temp2, temp3, temp4, temp5, temp6;

# surface is described by
# 	x = sin(u);
#       y = sin(v);
#       z = sin((dd - aa * u - bb * v) / cc);

# sin(x) = x - 1/6 * x^3 + 1/120 * x^5 - 1/5040 x^7 + 1/362880 * x^9......
MUL	temp0,  vPos,  vPos;	# temp0 = xyz^2

# compute the power functions
MUL	temp1,  vPos, temp0;  	# temp1 = xyz^3		n=2
MUL	temp2, temp1, temp0;  	# temp2 = xyz^5		n=3
MUL	temp3, temp2, temp0;  	# temp3 = xyz^7		n=4
MUL	temp4, temp3, temp0;  	# temp4 = xyz^9		n=5
MUL	temp5, temp4, temp0;  	# temp4 = xyz^11	n=6
MUL	temp6, temp5, temp0;  	# temp4 = xyz^13	n=7

# compute the sine.....
MAD	tPos, temp1,        one6, vPos;	# temp0 = x - 1/6 * x^3
MAD	tPos, temp2,      one120, tPos;	# temp0 = x - 1/6 * x^3 + 1/120 * x^5
MAD	tPos, temp3,     one5040, tPos;	# temp0 = x - 1/6 * x^3 + 1/120 * x^5 - 1/5040 * x^7
MAD	tPos, temp4,   one362880, tPos;	# temp0 = x - 1/6 * x^3 + 1/120 * x^5 - 1/5040 * x^7 + 1/362880 * x^9
MAD	tPos, temp5, one39916800, tPos;	# temp0 = x - 1/6 * x^3 + 1/120 * x^5 - 1/5040 * x^7 + 1/362880 * x^9....
MAD	tPos, temp6,  oneHoleLot, tPos;	# temp0 = x - 1/6 * x^3 + 1/120 * x^5 - 1/5040 * x^7 + 1/362880 * x^9....... = sin(x)

# Set the w to 1
SWZ	temp0, tPos, x,y,z,1;

# Transform the vertex by the modelview/projection matrix
DP4    tPos.x, state.matrix.mvp.row[0], temp0;
DP4    tPos.y, state.matrix.mvp.row[1], temp0;
DP4    tPos.z, state.matrix.mvp.row[2], temp0;
DP4    tPos.w, state.matrix.mvp.row[3], temp0;

# Set Position
MOV 	result.position, tPos;

# FABS
MUL	temp1, temp0, negOne;
MAX	temp0, temp0, temp1;

# Set Color
MOV	result.color, temp0;

END
