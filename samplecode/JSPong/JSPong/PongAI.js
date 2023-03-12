var lastBallY = 0;

function nextMove(paddle, ball)
{
    var ballY = ball.middleY;
    var ballDirection = ballY > lastBallY
        ? kUpDirection 
        : kDownDirection;
    lastBallY = ballY;

    if (ballY > paddle.top)
        return kUpDirection;

    if (ballY < paddle.bottom)
        return kDownDirection;

    return ballDirection;
}
