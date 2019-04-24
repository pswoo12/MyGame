/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

using namespace CocosDenshion;

#define BACKGROUND_MUSIC_SFX  "background-music-aac.mp3"
#define PEW_PEW_SFX           "pew-pew-lei.mp3"

// 글쓴이는 이렇게 하였다.
enum class PhysicsCategory {
	None = 0,
	Monster = (1 << 0),    // 1
	Projectile = (1 << 1), // 2
	All = PhysicsCategory::Monster | PhysicsCategory::Projectile // 3
};
//enum class PhysicsCategory {
//	None = 0,
//	Monster = 1,
//	Projectile = 2,
//	All = 3
//};

Scene* HelloWorld::createScene()
{
	// 코코스2d의 물리 시뮬레이션 제어.
	auto scene = Scene::createWithPhysics();
	// 기존에 return HelloWorld::create(); 부분을 레이어로 만들어주고
	auto layer = HelloWorld::create();
	scene->getPhysicsWorld()->setGravity(Vec2(0, 0));
	// 디버그를 통하여 제대로 작동하는지 볼 수 있다.
	scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
	// 레이어의 Child로 씬을 추가해주자.
	scene->addChild(layer);

	return scene;
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first Node 클래스를 상속받은 씬의 초기화를 불러줍니다.
    if ( !Scene::init() )
    {
        return false;
    }

	// 2. Director는 싱글톤으로 사용합니다.
	// Returns visible size of the OpenGL view in points
    auto visibleSize = Director::getInstance()->getVisibleSize();
	// Returns visible origin coordinate of the OpenGL view in points.
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// 3. DrawNode 회색으로(Color4F) 스크린을 채우고 씬에 백그라운드로써 추가해줍니다.
	// Faster than the "drawing primitives" since they draws everything in one single batch.
	// 먼저 DrawNode 객체를 생성합니다.
	auto background = DrawNode::create();
	// 그다음 drawSolidRect 함수를 활용하여 게임 화면을 회색으로 채웁니다.
	background->drawSolidRect(origin, visibleSize, Color4F(0.6, 0.6, 0.6, 1.0));
	// 그리고 씬에 백그라운드를 추가해줍니다.
	this->addChild(background);

	// 4. 스프라이트 객체를 생성해줍니다.
	m_sPlayer = Sprite::create("player.png");
	// 스프라이트 객체의 위치 설정을 해주고
	m_sPlayer->setPosition(Vec2(visibleSize.width * 0.1, visibleSize.height * 0.5));
	// 씬에 스프라이트 객체를 추가해줍니다.
	this->addChild(m_sPlayer);
	
	// 5. 랜덤 수를 생성하여 addMonster를 1.5초마다 생성해줍니다.
	srand((unsigned int)time(nullptr));
	// 1.5초마다 addMonster 함수포인터의 호출
	this->schedule(schedule_selector(HelloWorld::addMonster), 1.5);

    // 6. 3.x 버전부터 생긴 클릭 혹은 키보드 이벤트등을 받을 수 있는 기능인거 같습니다.
	// 이 부분은 본문의 내용을 좀 더 참고하는게 좋을듯.
	// EventDispatcher 객체의 기능들을 사용.
	// EventListenerTouchOneByOne 이것과 EventListenerTouchAllAtOnce
	// 두 종류가 있는거 같습니다. 하나의 터치이벤트를 받느냐, 모든 터치 이벤트를 받느냐 차이

	auto eventListener = EventListenerTouchOneByOne::create();
	eventListener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(eventListener, m_sPlayer);


	// 7. 충돌된 노드의 전달 
	// 6번은 터치에 관련된 이벤트라면 7번
	// EventListenerPhysicsContact 는 물리 이벤트로서
	// 비트마스크와 매치시켜 작동하게 된다.
	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(HelloWorld::onContactBegan, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, this);

	// 8. 키보드 이벤트(케릭터 움직이기)
	auto keyboardListener = EventListenerKeyboard::create();
	keyboardListener->onKeyPressed = CC_CALLBACK_2(HelloWorld::onKeyPressed, this);
	keyboardListener->onKeyReleased = CC_CALLBACK_2(HelloWorld::onKeyReleased, this);

	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyboardListener, m_sPlayer);

	m_isLeft = m_isRight = m_isDown = m_isUp = false;
	this->schedule(schedule_selector(HelloWorld::keyUpdate));

	// end. 백그라운드 뮤직
	SimpleAudioEngine::getInstance()->playBackgroundMusic(BACKGROUND_MUSIC_SFX, true);

	/////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

	/*
    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

    if (closeItem == nullptr ||
        closeItem->getContentSize().width <= 0 ||
        closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width/2;
        float y = origin.y + closeItem->getContentSize().height/2;
        closeItem->setPosition(Vec2(x,y));
    }

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label

    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    if (label == nullptr)
    {
        problemLoading("'fonts/Marker Felt.ttf'");
    }
    else
    {
        // position the label on the center of the screen
        label->setPosition(Vec2(origin.x + visibleSize.width/2,
                                origin.y + visibleSize.height - label->getContentSize().height));

        // add the label as a child to this layer
        this->addChild(label, 1);
    }

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");
    if (sprite == nullptr)
    {
        problemLoading("'HelloWorld.png'");
    }
    else
    {
        // position the sprite on the center of the screen
        sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

        // add the sprite as a child to this layer
        this->addChild(sprite, 0);
    }
	*/


    return true;
}

void HelloWorld::addMonster(float dt) {
	auto monster = Sprite::create("monster.png");

	//총알과 충돌 관련 start
	// 1 물리시뮬레이션, 객체를 박스로 감싼다.
	auto monsterSize = monster->getContentSize();
	auto physicsBody = PhysicsBody::createBox(Size(monsterSize.width, monsterSize.height),
		PhysicsMaterial(0.1f, 1.0f, 0.0f));
	// 2 동적제어
	physicsBody->setDynamic(true);
	// 3 위에 설정한 enum 데이터.
	physicsBody->setCategoryBitmask((int)PhysicsCategory::Monster);
	physicsBody->setCollisionBitmask((int)PhysicsCategory::None);
	physicsBody->setContactTestBitmask((int)PhysicsCategory::Projectile);

	monster->setPhysicsBody(physicsBody);
	//총알과 충돌 관련 end

	// 1. 몬스터를 랜덤 y좌표에 생성해줍니다.
	// 몬스터 이미지의 사이즈를 불러옵니다 대략 x : 25, y : 37
	auto monsterContentSize = monster->getContentSize();
	// 480, 320 의 씬과 관련된 사이즈값.
	auto selfContentSize = this->getContentSize();

	// 화면 위에서 몬스터 사이즈 반인 곳에서 시작하여
	int minY = monsterContentSize.height / 2;
	// 화면 가장 끝에서 몬스터 사이즈 반만큼 뺀 곳 까지
	int maxY = selfContentSize.height - monsterContentSize.height/2;
	// 이 y축 사이를 범위로
	int rangeY = maxY - minY;
	// 랜덤한 위치를 생성한다.
	int randomY = (rand() % rangeY) + minY;

	//몬스터의 x축은 화면 끝에서, y축은 랜덤하게 하여 몬스터의 위치를 결정하고
	monster->setPosition(Vec2(selfContentSize.width + monsterContentSize.width / 2, randomY));
	//해당 몬스터의 스프라이트를 추가해준다.
	this->addChild(monster);

	// 2. 몬스터의 랜덤 스피드를 결정.
	int minDuration = 2.0;
	int maxDuration = 4.0;
	int rangeDuration = maxDuration - minDuration;
	int randomDuration = (rand() % rangeDuration) + minDuration;

	// 3. 몬스터를 우측 끝에서 왼쪽으로 움직이게 해줍니다.
	// 이를 위해 MoveTo 객체를 생성해주고 (duration Time, vec2(x,y))
	auto actionMove = MoveTo::create(randomDuration, Vec2(-monsterContentSize.width / 2, randomY));
	// 이 MoveTo 객체를 자동으로 릴리즈 해주는 remove 객체도 생성해줍니다.
	auto actionRemove = RemoveSelf::create();
	// sprite는 Node 객체를 상속받아 만들어진 클래스인데 
	// 이 Node 객체에는 runAction 이라는 객체의 이동과 관련된 함수가 있습니다.
	// Action 이라는 객체를 받아서 실행 되는데 이 때 Action의 여러 종류들이 있습니다.
	// 여기서는 쭈우욱 흘러가는 Sequence 객체의 이동법을 사용하게 됩니다.
	// 이 Sequence는 화면을 횡단하게 되고 화면을 지나게 되면 actionRemove를 통해 자동으로 소멸됩니다.
	monster->runAction(Sequence::create(actionMove, actionRemove, nullptr));
}

bool HelloWorld::onTouchBegan(Touch *touch, Event *unused_event) {
	// 1  - Just an example for how to get the  _player object
	// addEventListenerWithSceneGraphPriority(eventListener, _player)
	//auto node = unused_event->getCurrentTarget();

	// 2 플레이어 터치의 범위 offset
	Vec2 touchLocation = touch->getLocation();
	Vec2 offset = touchLocation - m_sPlayer->getPosition();

	// 3 만약 x좌표가 플레이어 뒤라면 뒤로는 총을 못쏘게
	if (offset.x < 0) {
		return true;
	}

	// 4 총알의 Sprite 객체와 포지션 등록.
	auto projectile = Sprite::create("projectile.png");
	projectile->setPosition(m_sPlayer->getPosition());
	this->addChild(projectile);

	//몬스터와 충돌 관련 start
	auto projectileSize = projectile->getContentSize();
	auto physicsBody = PhysicsBody::createCircle(projectileSize.width / 2);
	physicsBody->setDynamic(true);
	physicsBody->setCategoryBitmask((int)PhysicsCategory::Projectile);
	physicsBody->setCollisionBitmask((int)PhysicsCategory::None);
	physicsBody->setContactTestBitmask((int)PhysicsCategory::Monster);

	projectile->setPhysicsBody(physicsBody);
	//몬스터와 충돌 관련 end

	// 5 offset은 Vec2 객체로써 
	// normalize()는 벡터의 x,y좌표의 값을 정규화 시켜줍니다.
	offset.normalize();
	auto shootAmount = offset * 1000;

	// 6 총알의 벡터값
	auto realDest = shootAmount + projectile->getPosition();

	// 7 총알의 속도는 일정하게 발사 시켜줍니다.
	// addMonster 부분에서 설명하였으니 생략.
	auto actionMove = MoveTo::create(2.0f, realDest);
	auto actionRemove = RemoveSelf::create();
	projectile->runAction(Sequence::create(actionMove, actionRemove, nullptr));

	// 8 총소리
	SimpleAudioEngine::getInstance()->playEffect(PEW_PEW_SFX);

	return true;
}

bool HelloWorld::onContactBegan(PhysicsContact &contact) {
	//충돌 된 노드들을 가져오게 됩니다.
	auto nodeA = contact.getShapeA()->getBody()->getNode();
	auto nodeB = contact.getShapeB()->getBody()->getNode();

	
	//가져온 노드의 삭제를 진행.
	// 글쓴이의 글에는 안전장치가 없어서 nullptr인 노드를 지우게 되면
	// 에러가 발생한다. 반드시 아래의 if문을 추가하도록.
	if(nodeA != nullptr)
		nodeA->removeFromParent();
	
	if (nodeB != nullptr)
		nodeB->removeFromParent();

	return true;
}

void HelloWorld::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event) {

	switch (keyCode) {
		case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
		case EventKeyboard::KeyCode::KEY_A:
			m_isLeft = true;
			break;
		case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
		case EventKeyboard::KeyCode::KEY_D:
			m_isRight = true;
			break;
		case EventKeyboard::KeyCode::KEY_UP_ARROW:
		case EventKeyboard::KeyCode::KEY_W:
			m_isUp = true;
			break;
		case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
		case EventKeyboard::KeyCode::KEY_S:
			m_isDown = true;
			break;
	}

}

void HelloWorld::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event) {
	
	switch (keyCode) {
		case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
		case EventKeyboard::KeyCode::KEY_A:
			m_isLeft = false;
			break;
		case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
		case EventKeyboard::KeyCode::KEY_D:
			m_isRight = false;
			break;
		case EventKeyboard::KeyCode::KEY_UP_ARROW:
		case EventKeyboard::KeyCode::KEY_W:
			m_isUp = false;
			break;
		case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
		case EventKeyboard::KeyCode::KEY_S:
			m_isDown = false;
			break;
	}
}

void HelloWorld::keyUpdate(float fDelta) {
	auto playerContentSize = m_sPlayer->getContentSize();
	float size = playerContentSize.width / 4;

	if (m_isLeft) {
		m_sPlayer->setPosition(m_sPlayer->getPosition() + Vec2(-size, 0));
	}
	else if (m_isRight) {
		m_sPlayer->setPosition(m_sPlayer->getPosition() + Vec2(size, 0));
	}
	else if (m_isUp) {
		m_sPlayer->setPosition(m_sPlayer->getPosition() + Vec2(0, size));
	}
	else if (m_isDown) {
		m_sPlayer->setPosition(m_sPlayer->getPosition() + Vec2(0, -size));
	}

}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);


}
