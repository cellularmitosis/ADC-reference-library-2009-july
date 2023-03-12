!!ARBvp1.0

ATTRIB vertexPosition  = vertex.position;
ATTRIB vertexTexcoord  = vertex.texcoord;

# Get simTime as a environment value
PARAM simTime		= program.env[0];

PARAM EmitterPosX	= {   0, 100, 200,   0};
PARAM EmitterPosY   = {   0,  60, 150,  50};
PARAM EmitterAmp    = {   5,   8,   3,   2};
PARAM EmitterFreq   = { 1.2, 0.8, 0.3, 2.0};
PARAM EmitterDecay	= {  20,  40,  10,  20};

# some standard and custom parameters that follow me around
PARAM negOne	= {-1.0, -1.0, -1.0, -1.0};
PARAM epsilon	= { 5.0E-6, 5.0E-6, 5.0E-6, 5.0E-6};
PARAM ptOne		= { 0.1, 0.1, 0.1, 0.1};
PARAM one		= { 1.0, 1.0, 1.0, 1.0};
PARAM _Pi		= { 3.1415926535, 3.1415926535, 3.1415926535, 3.1415926535};
PARAM invPi		= { 0.3183098861, 0.3183098861, 0.3183098861, 0.3183098861};
PARAM _2Pi		= { 6.2831853071, 6.2831853071, 6.2831853071, 6.2831853071};
PARAM inv2Pi	= { 0.1591549439, 0.1591549439, 0.1591549439, 0.1591549439};
PARAM ten		= { 10,           10,           10,           10};

# Factorial table for computing cosine and exp using sum of products functions
PARAM InvFact_2		= {	2,					-2,					0.500000000000000, -0.500000000000000};
PARAM InvFact_3		= {	6,					-6,					0.166666666666667, -0.166666666666667};
PARAM InvFact_4		= {	24,					-24,				0.041666666666667, -0.041666666666667};
PARAM InvFact_5		= {	120,				-120,				0.008333333333333, -0.008333333333333};
PARAM InvFact_6		= {	720,				-720,				0.001388888888889, -0.001388888888889};
PARAM InvFact_7		= {	5040,				-5040,				1.98412698413e-04, -1.98412698413e-04};
PARAM InvFact_8		= {	40320,				-40320,				2.48015873016e-05, -2.48015873016e-05};
PARAM InvFact_10	= {	3.6288e06,		   	-3.6288e06,			2.75573192240e-07, -2.75573192240e-07};
PARAM InvFact_12	= {	4.79002e08,		   	-4.79002e08,		2.08767569879e-09, -2.08767569879e-09};
PARAM InvFact_14	= {	8.71783e10,		   	-8.71783e10,		1.14707455977e-11, -1.14707455977e-11};
PARAM InvFact_16	= {	2.09228e13,		   	-2.09228e13,		4.77947733239e-14, -4.77947733239e-14};

PARAM select		= {1, 0, 0, 0};

TEMP	dist, cos, exp, pos, temp0, temp1, temp2, temp3;
                        
# compute distance from emitter
# xVec = vec_sub(emitterX[k], vec_ld(0, (vector float *)&xPos[ii + j]));
# yVec = vec_sub(emitterY[k], vec_ld(0, (vector float *)&yPos[ii + j]));

MAD	temp0, negOne, vertexPosition.xxxx, EmitterPosX;	# temp0 = Emitter.position.x - position.x
MAD	temp1, negOne, vertexPosition.yyyy, EmitterPosY;	# temp1 = Emitter.position.y - position.y

# dist = sqrt(x*x + y*y) * 0.1;
MUL temp0, temp0, temp0;
MUL temp1, temp1, temp1;
ADD	dist, temp0, temp1;

RSQ	temp0, dist.x;
RSQ	temp1, dist.y;
RSQ	temp2, dist.z;
RSQ	temp3, dist.w;
RCP	temp0, temp0.x;		# temp0 = dist0
RCP	temp1, temp1.x;		# temp1 = dist1
RCP	temp2, temp2.x;		# temp2 = dist2
RCP	temp3, temp3.x;		# temp3 = dist3

# put temp0, temp1, temp2 and temp3 into temp0.xyzw
MUL	temp0, temp0, select.xwww;
MAD	temp0, temp1, select.wxww, temp0;
MAD	temp0, temp2, select.wwxw, temp0;
MAD	temp0, temp3, select.wwwx, temp0;

# temp0 = dist * 0.1
MUL	dist, temp0, ptOne;

# this is the emitter function we have to compute
# z += amp * cos(freq * dist + sim_time) / exp(decay / dist);

# compute freq * dist + sim_time
MAD	temp0, EmitterFreq, dist, simTime.xxxx;

# bound tempVec to 0 <-> 2 pi
MUL	temp0, temp0, inv2Pi;	# div by 2 Pi
FRC	temp0, temp0;		# take frational result (opposite of mod)
MUL	temp0, temp0, _2Pi;	# mult fractional result by 2 Pi to get bounded value

# temp0 is now between -pi and pi

# compute cos(freq * dist + sim_time)
# Cos = sum (-1)^n*x^2n/2n!
# compute sum of products for cos(freq * dist + sim_time)
MUL	temp1, temp0, temp0;				# temp1 = x^2;
MAD	cos,  InvFact_2.wwww, temp1, one;	# cos = 1 - 1/fact(2)*x^2

MUL	temp0, temp1, temp1;				# temp0 = x^4, temp1 = x^2;
MAD	cos,  InvFact_4.zzzz, temp0, cos;	# cos = 1 - 1/fact(2)*x^2 + 1/fact(4)*x^4

MUL	temp0, temp0, temp1;				# temp0 = x^6, temp1 = x^2;
MAD	cos,  InvFact_6.wwww, temp0, cos;	# cos = 1 - 1/fact(2)*x^2 + 1/fact(4)*x^4 - 1/fact(6)*x^6

MUL	temp0, temp0, temp1;				# temp0 = x^8, temp1 = x^2;
MAD	cos,  InvFact_8.zzzz, temp0, cos;	# cos = 1 - 1/fact(2)*x^2 + 1/fact(4)*x^4 - 1/fact(6)*x^6 + 1/fact(8)*x^8

MUL	temp0, temp0, temp1;				# temp0 = x^10, temp1 = x^2;
MAD	cos, InvFact_10.wwww, temp0, cos;	# cos = 1 - 1/fact(2)*x^2 + 1/fact(4)*x^4 - 1/fact(6)*x^6 + 1/fact(8)*x^8 - 1/fact(10)*x^10

MUL	temp0, temp0, temp1;				# temp0 = x^12, temp1 = x^2;
MAD	cos, InvFact_12.zzzz, temp0, cos;	# cos = 1 - 1/fact(2)*x^2 + 1/fact(4)*x^4 - 1/fact(6)*x^6 + 1/fact(8)*x^8 - 1/fact(10)*x^10 +
										# 		1/fact(12)*x^12

MUL	temp0, temp0, temp1;				# temp0 = x^14, temp1 = x^2;
MAD	cos, InvFact_14.wwww, temp0, cos;	# cos = 1 - 1/fact(2)*x^2 + 1/fact(4)*x^4 - 1/fact(6)*x^6 + 1/fact(8)*x^8 - 1/fact(10)*x^10 + 
										# 		1/fact(12)*x^12 - 1/fact(14)*x^14

MUL	temp0, temp0, temp1;				# temp0 = x^16, temp1 = x^2;
MAD	cos, InvFact_16.zzzz, temp0, cos;	# cos = 1 - 1/fact(2)*x^2 + 1/fact(4)*x^4 - 1/fact(6)*x^6 + 1/fact(8)*x^8 - 1/fact(10)*x^10 +
										# 		1/fact(12)*x^12 - 1/fact(14)*x^14 + 1/fact(16)*x^16

# now we have the cos(freq * dist + sim_time)
MIN	cos, cos, one;

# amp * cos(freq * dist + sim_time)
MUL	cos, EmitterAmp, cos;

# compute decay / dist
MAD	temp0, negOne, vertexPosition.xxxx, EmitterPosX;	# temp0 = Emitter.position.x - position.x
MAD	temp1, negOne, vertexPosition.yyyy, EmitterPosY;	# temp1 = Emitter.position.y - position.y

# dist = sqrt(x*x + y*y);
MUL temp0, temp0, temp0;
MUL temp1, temp1, temp1;
ADD	dist, temp0, temp1;

RSQ	temp0, dist.x;
RSQ	temp1, dist.y;
RSQ	temp2, dist.z;
RSQ	temp3, dist.w;

# put temp0, temp1, temp2 and temp3 into temp0
MUL	temp0, temp0, select.xwww;
MAD	temp0, temp1, select.wxww, temp0;
MAD	temp0, temp2, select.wwxw, temp0;
MAD	temp0, temp3, select.wwwx, temp0;

# compute decay / dist
MAX	temp0, temp0, epsilon;
MUL	temp0, EmitterDecay, temp0;

# now temp0 = decay / dist
ADD	exp, one, temp0;					# temp0 = x
										# exp = 1 + x
										
MUL	temp1, temp0, temp0;				# temp0 = x, temp1 = x^2
MAD	exp, temp1, InvFact_2.zzzz, exp;	# exp = 1 + x + 1/fact(2)*x^2

MUL	temp1, temp1, temp0;				# temp0 = x, temp1 = x^3
MAD	exp, temp1, InvFact_3.zzzz, exp;	# exp = 1 + x + 1/fact(2)*x^2 + 1/fact(3)*x^3

MUL	temp1, temp1, temp0;				# temp0 = x, temp1 = x^4
MAD	exp, temp1, InvFact_4.zzzz, exp;	# exp = 1 + x + 1/fact(2)*x^2 + 1/fact(3)*x^3 + 1/fact(4)*x^4

MUL	temp1, temp1, temp0;				# temp0 = x, temp1 = x^5
MAD	exp, temp1, InvFact_5.zzzz, exp;	# exp = 1 + x + 1/fact(2)*x^2 + 1/fact(3)*x^3 + 1/fact(4)*x^4 + 1/fact(5)*x^5

# compute 1 / exp
RCP	temp0, exp.x;
RCP	temp1, exp.y;
RCP	temp2, exp.z;
RCP	temp3, exp.w;

# put temp0, temp1, temp2 and temp3 into exp
MUL	temp0, temp0, select.xwww;
MAD	temp0, temp1, select.wxww, temp0;
MAD	temp0, temp2, select.wwxw, temp0;
MAD	temp0, temp3, select.wwwx, temp0;

# compute amp * cos(freq * dist + sim_time) / exp (decay / dist)
MUL	exp, temp0, cos;

# compute sum for z value
MOV	temp0, exp.xxxx;
ADD	temp0, temp0, exp.yyyy;
ADD	temp0, temp0, exp.zzzz;
#ADD	temp0, temp0, exp.wwww;		# use only 3 emitters, the host CPU does this also

SWZ	pos, vertexPosition, x,y,0,1;	# clear z
SWZ	temp0, temp0, 0,0,z,0;			# clear all but z

ADD	pos, pos, temp0;				# add z back in
SWZ	pos, pos, x,y,z,1;				# set w to 1

DP4	temp0.x, state.matrix.mvp.row[0], pos;
DP4	temp0.y, state.matrix.mvp.row[1], pos;
DP4	temp0.z, state.matrix.mvp.row[2], pos;
DP4	temp0.w, state.matrix.mvp.row[3], pos;

MOV	result.position, temp0;
MOV	result.texcoord, vertexTexcoord;

END
