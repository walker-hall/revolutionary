// model.cpp
#include "model.h"
#include "contactlistener.h"

Model::Model(QWidget *parent)
    : QWidget(parent)
    , world(nullptr)
    , worldContact(new ContactListener())
    , collisionObjects()
    , scene(width(), height())
    , painter(&scene)
    , elapsedTimer()
{
    b2Vec2 gravity(0.0f, -10.0);
    world = new b2World(gravity);
    world->SetContactListener(worldContact);

    worldState = { world, worldContact };

    // player
    player = new Player(QPoint(0, 0), &worldState);

    QVector<Wall> walls;
    QVector<Door> doors;

    Wall wall(0, 0, 0, 0, &worldState);

    // Create levels question
    Door door(QPoint(), wall, "George Washington was born in...", false);
    door.insertQuesionResponse("March 11, 1690", false);
    door.insertQuesionResponse("April 20, 1215", false);
    door.insertQuesionResponse("February 22, 1732", true);
    door.insertQuesionResponse("December 9, 2025", false);


    door.insertHint("George Washington had no biological children.");
    door.insertHint("George Washingtons dentures\n were not made of wood.");
    door.insertHint("George Washington loved to party.");
    door.insertHint("Hint 4");


    walls.append(wall);

    doors.append(door);

    Level level1(walls, doors);
    levels.append(level1);

    // createCollisionObject(QPoint(0, 100), QPoint(20, 100), 20);
    createCollisionObject(0., -50., 50., 5.);

    connect(&worldTimer, &QTimer::timeout, this, &Model::worldStep);
    worldTimer.start();
    elapsedTimer.start();
}

Model::~Model()
{
    delete world;
}

// void Model::createCollisionObject(QPoint p1, QPoint p2, int width)
// {
//     Wall newWall(p1, p2, width, world);
//     collisionObjects.push_back(newWall);
// }

void Model::createCollisionObject(int x, int y, int width, int height)
{
    Wall newWall(x, y, width, height, &worldState);
    collisionObjects.push_back(newWall);
}


void Model::worldStep()
{
    if (!world) {
        return;
    }

    qint64 currentTime = elapsedTimer.nsecsElapsed();
    qint64 elapsedTime = currentTime - lastFrameTime;

    if (elapsedTime >= FRAME_TIME_TARGET) {
        lastFrameTime = currentTime;

        player->step();
        world->Step(1.0f / 60.0f, 8, 3);

        qreal frameRate = 1e9 / static_cast<qreal>(elapsedTime);
        int roundedFrameRate = qRound(frameRate);
        // qDebug() << "Frame Rate:" << roundedFrameRate << "FPS";
    }

    renderScene();
}

void Model::renderScene() {
    scene.fill(Qt::white);

    painter.setPen(Qt::NoPen);

    painter.setBrush(Qt::blue);
    // b2Vec2 playerPosition = SCALE_FACTOR * player->getTopLeft();
    // QImage sprite(":/rightWalking0");
    // sprite.mirror(false, true);
    // painter.save();

    // painter.drawImage(QRect(playerPosition.x, playerPosition.y, player->width * SCALE_FACTOR, player->height * SCALE_FACTOR),
    //                   sprite,
    //                   QRect(0, 6, 100, 100));
    // painter.restore();
    //painter.drawRect(playerPosition.x, playerPosition.y, player->width * SCALE_FACTOR, player->height * SCALE_FACTOR);

    player->render(&painter);
    // Render the walls
    painter.setBrush(Qt::red);
    for (const Wall& wall : collisionObjects) {
        painter.drawRect(wall.x * SCALE_FACTOR, wall.y * SCALE_FACTOR, wall.width * SCALE_FACTOR, wall.height * SCALE_FACTOR);
    }

    QTransform transform;
    transform.scale(1.0, -1.0);
    painter.setTransform(transform);

    emit renderSceneOnView(scene);
}

void Model::onPlayerMoveState(Player::Movement state, bool isDown) {
    player->setMoveState(state, isDown);
}

void Model::onDoorCollisionState() {
    QString doorQuestionText = levels.at(currentLevel).doors.at(0).questionText;

    QHash<QString, bool> questionResponses = levels.at(currentLevel).doors.at(0).questionResponses;

    emit generateQuestionnaire(doorQuestionText, questionResponses);
}

void Model::isInputCorrect(QString response) {
    Level level = levels.at(currentLevel);

    bool isCorrect = level.isCorrectResponse(response, 0);
    QString messagePopUp = level.generateHintResponse(response, 0);
    emit displayPopUp(isCorrect, messagePopUp);
}
