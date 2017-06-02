#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;
constexpr int windowWidth{800},windowHeight{600};
constexpr float ballRadius{10.f},ballVelocity{8.f};//f declares 32bit float
constexpr float paddleWidth{100.f},paddleHeight{8.f}, paddleVelocity{6.f};
constexpr float brickWidth{60.f},brickHeight{20.f};
constexpr int countBlocksX{11}, countBlocksY{4};
/*
 * ball definition
 */
struct Ball{
    CircleShape shape;
    Vector2f velocity{-ballVelocity, -ballVelocity};
    Ball(float x, float y){
        shape.setPosition(x,y);
        shape.setRadius(ballRadius);
        shape.setFillColor(Color::Yellow);
        shape.setOrigin(ballRadius,ballRadius);
    }
    //position methods
    float x(){ return shape.getPosition().x;}
    float y(){ return shape.getPosition().y;}
    float left(){ return x()-shape.getRadius();}
    float right(){ return x()+shape.getRadius();}
    float up(){ return y()-shape.getRadius();}
    float down(){ return y()+shape.getRadius();}
    void update(){
        //move the shape
        shape.move(velocity);
        //wall detection nd collision handling
        if(left()<0)velocity.x=ballVelocity;
        else if(right()>windowWidth)velocity.x=-ballVelocity;
        if(up()<0)velocity.y=ballVelocity;
        else if(down()>windowHeight)velocity.y=-ballVelocity;
    }
};
struct Paddle{
    RectangleShape shape;
    Vector2f velocity;
    Paddle(float x, float y){
        shape.setPosition(x,y);
        shape.setSize({paddleWidth,paddleHeight});
        shape.setFillColor(Color::White);
        shape.setOrigin(paddleWidth/2.f,paddleHeight/2.f);
    }
    float x(){ return shape.getPosition().x;}
    float y(){ return shape.getPosition().y;}
    float left(){ return x()-shape.getSize().x/2.f;}
    float right(){ return x()+shape.getSize().x/2.f;}
    float up(){ return y()-shape.getSize().y/2.f;}
    float down(){ return y()+shape.getSize().y/2.f;}
    void update(){
        shape.move(velocity);
        if(Keyboard::isKeyPressed(Keyboard::Key::A)&&left()>0)velocity.x=-paddleVelocity;
        else if(Keyboard::isKeyPressed(Keyboard::Key::D)&&right()<windowWidth)velocity.x=paddleVelocity;
        else velocity.x = 0;
    }
};
struct Brick{
    bool destroyed{false};
    RectangleShape shape;
    Brick(float x, float y){
        shape.setPosition(x,y);
        shape.setSize({paddleWidth,paddleHeight});
        shape.setFillColor(Color::Yellow);
        shape.setOrigin(brickWidth/2.f,brickHeight/2.f);
    }
    float x(){ return shape.getPosition().x;}
    float y(){ return shape.getPosition().y;}
    float left(){ return x()-shape.getSize().x/2.f;}
    float right(){ return x()+shape.getSize().x/2.f;}
    float up(){ return y()-shape.getSize().y/2.f;}
    float down(){ return y()+shape.getSize().y/2.f;}
};
template <class T1, class T2> bool isIntersecting(T1& mA, T2& mB) {
    return mA.right() >= mB.left() && mA.left() <= mB.right() &&
           mA.down() >= mB.up() && mA.up() <= mB.down();
}

void testCollision(Paddle& mPaddle, Ball& mBall) {
    if(!isIntersecting(mPaddle, mBall)) return;
    // otherwise bounce the ball
    mBall.velocity.y = -ballVelocity;

    // And let's direct it dependently on the position where the
    // paddle was hit.
    if(mBall.x() < mPaddle.x())
        mBall.velocity.x = -ballVelocity;
    else
        mBall.velocity.x = ballVelocity;
}
int main()
{
    RenderWindow window({windowWidth,windowHeight}, "super arkanoid");
    window.setFramerateLimit(30);
    window.setVerticalSyncEnabled(true);
    Ball ball{windowWidth/2,windowHeight/2};
    Paddle paddle{windowWidth/2,windowHeight-50};
    //game loop
    while(true){
        //clear window
        window.clear(Color::Black);
        if(Keyboard::isKeyPressed(Keyboard::Key::Escape))
            break;
        vector<Brick> bricks;
        for(int x=0; x<countBlocksX;++x){
            for(int y=0; y<countBlocksY;++y){
                bricks.emplace_back(
                        (x + 1) * (brickWidth + 3) + 22, (y + 2) * (brickHeight + 3));//fix// x??
            }
        }
        ball.update();
        paddle.update();
        testCollision(paddle,ball);
        window.draw(ball.shape);
        window.draw(paddle.shape);
        for(auto& brick: bricks)
            window.draw(brick.shape);
        window.display();
    }
    return 0;
}