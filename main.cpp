#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace sf;
/*
 * An enhanced version of arkanoid using SFML: credit to https://github.com/SuperV1234 for his tutorial for SFML
 *
 * todo files/ leaderboards/ powerups
 */
constexpr int windowWidth{800},windowHeight{600};
constexpr float ballRadius{10.f},ballVelocity{8.f};//f declares 32bit float
constexpr float paddleWidth{140.f},paddleHeight{8.f}, paddleVelocity{10.f};
constexpr float brickWidth{60.f},brickHeight{20.f};
constexpr int countBlocksX{11}, countBlocksY{4};
RenderWindow window({windowWidth,windowHeight}, "super arkanoid");
int lives{3};
int scoreVal{0};
int combo{1};
bool submitted{false};
string str;
Font font;
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
        switch(lives){
            case 3:shape.setFillColor(Color::Yellow);
                break;
            case 2:shape.setFillColor(Color::Blue);
                break;
            case 1:shape.setFillColor(Color::Green);
                break;
            default:
                break;
        }
        //wall detection nd collision handling
        if(left()<0)velocity.x=ballVelocity;
        else if(right()>windowWidth)velocity.x=-ballVelocity;
        if(up()<0)velocity.y=ballVelocity;
        else if(down()>windowHeight){
            velocity.y=-ballVelocity;
            lives--;
            combo=1;
            scoreVal/=2;
        }
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
        shape.setSize({brickWidth,brickHeight});
        shape.setFillColor(Color::White);
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
struct Score{
    int score;
    string name;
    Score(int scoreIn, string nameIn){
        score = scoreIn;
        name = nameIn;
    }
    Score(){
        score = 0;
        name = "-";
    }
    string printScore(){
        return name + " - " + to_string(score) +"\n";
    }
};
Score scoreList[8];
void testCollision(Paddle& mPaddle, Ball& mBall) {
    if(!isIntersecting(mPaddle, mBall)) return;
    combo=1;
    // otherwise bounce the ball
    mBall.velocity.y = -ballVelocity;

    // And let's direct it dependently on the position where the
    // paddle was hit.
    if(mBall.x() < mPaddle.x())
        mBall.velocity.x = -ballVelocity/(mPaddle.x()/mBall.x());
    else
        mBall.velocity.x = ballVelocity/(mBall.x()/mPaddle.x());
}
void testCollision(Brick& mBrick, Ball& mBall) {
    if(!isIntersecting(mBrick, mBall)) return;
    mBrick.destroyed=true;
    combo++;
    scoreVal+=50+(combo-2)*10;
    float overlapLeft{mBall.right() - mBrick.left()};
    float overlapRight{mBrick.right() - mBall.left()};
    float overlapTop{mBall.down() - mBrick.up()};
    float overlapBottom{mBrick.down() - mBall.up()};
    //find the magnitude of overlaps and make a boolean direction for the ball
    bool ballFromLeft(abs(overlapLeft) < abs(overlapRight));
    bool ballFromTop(abs(overlapTop) < abs(overlapBottom));
    // Let's store the minimum overlaps for the X and Y axes.
    float minOverlapX{ballFromLeft ? overlapLeft : overlapRight};
    float minOverlapY{ballFromTop ? overlapTop : overlapBottom};
    //figure out whether collision was vertical or horizontal
    if(abs(minOverlapX) < abs(minOverlapY))
        mBall.velocity.x = ballFromLeft ? -ballVelocity : ballVelocity;
    else
        mBall.velocity.y = ballFromTop ? -ballVelocity : ballVelocity;
}
void playGame(){
    Ball ball{windowWidth/2,windowHeight/2};
    Paddle paddle{windowWidth/2,windowHeight-50};
    Text score;
    if (!font.loadFromFile("../Oxygen-Regular.ttf"))
    {
        cout << "Error loading font\n" ;
    }
    score.setScale(0.75,0.75);
    score.setPosition({windowWidth - 250, 3});
    score.setColor(Color::White);
    score.setFont(font);
    //make bricks
    vector<Brick> bricks;
    for(int x=0; x<countBlocksX;++x){
        for(int y=0; y<countBlocksY;++y){
            bricks.emplace_back(
                    (x + 1) * (brickWidth + 3) + 22, (y + 2) * (brickHeight + 3));//fix// x??
        }
    }
    //game loop
    while(lives>0){
        //clear window
        window.clear(Color::Black);
        if(Keyboard::isKeyPressed(Keyboard::Key::Escape)||bricks.empty())
            break;
        ball.update();
        paddle.update();
        //test collisions
        testCollision(paddle,ball);
        for(auto& brick : bricks) testCollision(brick, ball);

        // And we use the "erase-remove idiom" to remove all `destroyed`
        // blocks from the block vector - using a cool C++11 lambda!
        bricks.erase(remove_if(begin(bricks), end(bricks),
                           [](const Brick& mBrick)
                           {
                               return mBrick.destroyed;
                           }),
                 end(bricks));
        window.draw(ball.shape);
        window.draw(paddle.shape);
        //update and draw score
        score.setString("score: "+to_string(scoreVal) +" combo: "+to_string(combo));
        window.draw(score);
        for(auto& brick: bricks)
            window.draw(brick.shape);
        window.display();
    }
}
/*
 * update to use saving and loading
 */
void loadLeaderboard() {
    str = "nam";
    ifstream infile;
    infile.open("lboard.txt");
    if (infile.fail()) {
        scoreList[0] = {2000, "wow"};
        scoreList[1] = {1750, "dam"};
        scoreList[2] = {1500, "sly"};
        scoreList[3] = {1250, "hah"};
        scoreList[4] = {1000, "meh"};
        scoreList[5] = {750, "cry"};
        scoreList[6] = {500, "bad"};
        scoreList[7] = {250, "lsr"};
    } else{
        bool flipFlop = true;
        int i = 0;
        string tmp;
        for (std::string line; std::getline(infile, line); )
        {

            if(flipFlop){
                tmp = line;
            }else{
                int score = stoi(line);
                scoreList[i] = {score,tmp};
                i++;
            }
            flipFlop = !flipFlop;
        }
    }

}
void saveLeaderboard(){
    ofstream outfile;
    outfile.open("lboard.txt");
    outfile.clear();
    for(int i{0};i<8;i++) {
        outfile << scoreList[i].name << endl << scoreList[i].score<<endl;
    }
    outfile.close();
}
void leaderBoard(){
        while(true){
            window.clear(Color::Black);
            Text leaderboard;
            Text newScore;
            leaderboard.setColor(Color::White);
            leaderboard.setFont(font);
            newScore.setColor(Color::Yellow);
            newScore.setFont(font);
            string boardValues{"Leaderboard:\n\n"};
            for(int x{0};x< 8;x++){
                boardValues += scoreList[x].printScore();
            }
            leaderboard.setString(boardValues);
            leaderboard.setPosition({windowWidth/2-leaderboard.getLocalBounds().width/2, windowHeight/8});
            //set 3 character name
            Event event;
            if(window.pollEvent(event)){
                if (event.type == Event::TextEntered)
                {
                    // Handle ASCII characters only
                    if(event.text.unicode==8 && !submitted) {
                        if (str.length() > 0) {
                            str.pop_back();
                        }
                    }else if(event.text.unicode == 27) {
                        break;
                    }else if(event.text.unicode == 13 && !submitted){
                        //add score and break
                        for(int i{0}; i<8;i++){
                            if(scoreList[i].score<scoreVal){
                                for(int j{7}; j>i;j--) {
                                    scoreList[j] = scoreList[j-1];
                                }
                                scoreList[i]={scoreVal,str};
                                saveLeaderboard();
                                break;
                            }
                        }
                        submitted = true;
                        //break;
                    }else if (event.text.unicode < 128 && !submitted) {
                        if(str.length() >= 3){
                            str="";
                        }
                        if (str.length() < 3) {
                            str += static_cast<char>(event.text.unicode);
                        }
                    }
                }
            }
            Score newVal(scoreVal,str);
            newScore.setString(newVal.printScore());
            newScore.setPosition(leaderboard.getPosition().x,leaderboard.getPosition().y+leaderboard.getLocalBounds().height);
            window.draw(leaderboard);
            if(!submitted)
                window.draw(newScore);
            window.display();
        }
};
int main()
{
    window.setFramerateLimit(30);
    window.setVerticalSyncEnabled(true);
    loadLeaderboard();
    playGame();
    //empty event buffer
    Event event;
    while (window.pollEvent(event));
    leaderBoard();
    return 0;
}