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

// �۾��̴� �̷��� �Ͽ���.
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
	// ���ڽ�2d�� ���� �ùķ��̼� ����.
	auto scene = Scene::createWithPhysics();
	// ������ return HelloWorld::create(); �κ��� ���̾�� ������ְ�
	auto layer = HelloWorld::create();
	scene->getPhysicsWorld()->setGravity(Vec2(0, 0));
	// ����׸� ���Ͽ� ����� �۵��ϴ��� �� �� �ִ�.
	scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
	// ���̾��� Child�� ���� �߰�������.
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
    // 1. super init first Node Ŭ������ ��ӹ��� ���� �ʱ�ȭ�� �ҷ��ݴϴ�.
    if ( !Scene::init() )
    {
        return false;
    }

	// 2. Director�� �̱������� ����մϴ�.
	// Returns visible size of the OpenGL view in points
    auto visibleSize = Director::getInstance()->getVisibleSize();
	// Returns visible origin coordinate of the OpenGL view in points.
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// 3. DrawNode ȸ������(Color4F) ��ũ���� ä��� ���� ��׶���ν� �߰����ݴϴ�.
	// Faster than the "drawing primitives" since they draws everything in one single batch.
	// ���� DrawNode ��ü�� �����մϴ�.
	auto background = DrawNode::create();
	// �״��� drawSolidRect �Լ��� Ȱ���Ͽ� ���� ȭ���� ȸ������ ä��ϴ�.
	background->drawSolidRect(origin, visibleSize, Color4F(0.6, 0.6, 0.6, 1.0));
	// �׸��� ���� ��׶��带 �߰����ݴϴ�.
	this->addChild(background);

	// 4. ��������Ʈ ��ü�� �������ݴϴ�.
	m_sPlayer = Sprite::create("player.png");
	// ��������Ʈ ��ü�� ��ġ ������ ���ְ�
	m_sPlayer->setPosition(Vec2(visibleSize.width * 0.1, visibleSize.height * 0.5));
	// ���� ��������Ʈ ��ü�� �߰����ݴϴ�.
	this->addChild(m_sPlayer);
	
	// 5. ���� ���� �����Ͽ� addMonster�� 1.5�ʸ��� �������ݴϴ�.
	srand((unsigned int)time(nullptr));
	// 1.5�ʸ��� addMonster �Լ��������� ȣ��
	this->schedule(schedule_selector(HelloWorld::addMonster), 1.5);

    // 6. 3.x �������� ���� Ŭ�� Ȥ�� Ű���� �̺�Ʈ���� ���� �� �ִ� ����ΰ� �����ϴ�.
	// �� �κ��� ������ ������ �� �� �����ϴ°� ������.
	// EventDispatcher ��ü�� ��ɵ��� ���.
	// EventListenerTouchOneByOne �̰Ͱ� EventListenerTouchAllAtOnce
	// �� ������ �ִ°� �����ϴ�. �ϳ��� ��ġ�̺�Ʈ�� �޴���, ��� ��ġ �̺�Ʈ�� �޴��� ����

	auto eventListener = EventListenerTouchOneByOne::create();
	eventListener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(eventListener, m_sPlayer);


	// 7. �浹�� ����� ���� 
	// 6���� ��ġ�� ���õ� �̺�Ʈ��� 7��
	// EventListenerPhysicsContact �� ���� �̺�Ʈ�μ�
	// ��Ʈ����ũ�� ��ġ���� �۵��ϰ� �ȴ�.
	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(HelloWorld::onContactBegan, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, this);

	// 8. Ű���� �̺�Ʈ(�ɸ��� �����̱�)
	auto keyboardListener = EventListenerKeyboard::create();
	keyboardListener->onKeyPressed = CC_CALLBACK_2(HelloWorld::onKeyPressed, this);
	keyboardListener->onKeyReleased = CC_CALLBACK_2(HelloWorld::onKeyReleased, this);

	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyboardListener, m_sPlayer);

	m_isLeft = m_isRight = m_isDown = m_isUp = false;
	this->schedule(schedule_selector(HelloWorld::keyUpdate));

	// end. ��׶��� ����
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

	//�Ѿ˰� �浹 ���� start
	// 1 �����ùķ��̼�, ��ü�� �ڽ��� ���Ѵ�.
	auto monsterSize = monster->getContentSize();
	auto physicsBody = PhysicsBody::createBox(Size(monsterSize.width, monsterSize.height),
		PhysicsMaterial(0.1f, 1.0f, 0.0f));
	// 2 ��������
	physicsBody->setDynamic(true);
	// 3 ���� ������ enum ������.
	physicsBody->setCategoryBitmask((int)PhysicsCategory::Monster);
	physicsBody->setCollisionBitmask((int)PhysicsCategory::None);
	physicsBody->setContactTestBitmask((int)PhysicsCategory::Projectile);

	monster->setPhysicsBody(physicsBody);
	//�Ѿ˰� �浹 ���� end

	// 1. ���͸� ���� y��ǥ�� �������ݴϴ�.
	// ���� �̹����� ����� �ҷ��ɴϴ� �뷫 x : 25, y : 37
	auto monsterContentSize = monster->getContentSize();
	// 480, 320 �� ���� ���õ� �����.
	auto selfContentSize = this->getContentSize();

	// ȭ�� ������ ���� ������ ���� ������ �����Ͽ�
	int minY = monsterContentSize.height / 2;
	// ȭ�� ���� ������ ���� ������ �ݸ�ŭ �� �� ����
	int maxY = selfContentSize.height - monsterContentSize.height/2;
	// �� y�� ���̸� ������
	int rangeY = maxY - minY;
	// ������ ��ġ�� �����Ѵ�.
	int randomY = (rand() % rangeY) + minY;

	//������ x���� ȭ�� ������, y���� �����ϰ� �Ͽ� ������ ��ġ�� �����ϰ�
	monster->setPosition(Vec2(selfContentSize.width + monsterContentSize.width / 2, randomY));
	//�ش� ������ ��������Ʈ�� �߰����ش�.
	this->addChild(monster);

	// 2. ������ ���� ���ǵ带 ����.
	int minDuration = 2.0;
	int maxDuration = 4.0;
	int rangeDuration = maxDuration - minDuration;
	int randomDuration = (rand() % rangeDuration) + minDuration;

	// 3. ���͸� ���� ������ �������� �����̰� ���ݴϴ�.
	// �̸� ���� MoveTo ��ü�� �������ְ� (duration Time, vec2(x,y))
	auto actionMove = MoveTo::create(randomDuration, Vec2(-monsterContentSize.width / 2, randomY));
	// �� MoveTo ��ü�� �ڵ����� ������ ���ִ� remove ��ü�� �������ݴϴ�.
	auto actionRemove = RemoveSelf::create();
	// sprite�� Node ��ü�� ��ӹ޾� ������� Ŭ�����ε� 
	// �� Node ��ü���� runAction �̶�� ��ü�� �̵��� ���õ� �Լ��� �ֽ��ϴ�.
	// Action �̶�� ��ü�� �޾Ƽ� ���� �Ǵµ� �� �� Action�� ���� �������� �ֽ��ϴ�.
	// ���⼭�� �޿�� �귯���� Sequence ��ü�� �̵����� ����ϰ� �˴ϴ�.
	// �� Sequence�� ȭ���� Ⱦ���ϰ� �ǰ� ȭ���� ������ �Ǹ� actionRemove�� ���� �ڵ����� �Ҹ�˴ϴ�.
	monster->runAction(Sequence::create(actionMove, actionRemove, nullptr));
}

bool HelloWorld::onTouchBegan(Touch *touch, Event *unused_event) {
	// 1  - Just an example for how to get the  _player object
	// addEventListenerWithSceneGraphPriority(eventListener, _player)
	//auto node = unused_event->getCurrentTarget();

	// 2 �÷��̾� ��ġ�� ���� offset
	Vec2 touchLocation = touch->getLocation();
	Vec2 offset = touchLocation - m_sPlayer->getPosition();

	// 3 ���� x��ǥ�� �÷��̾� �ڶ�� �ڷδ� ���� �����
	if (offset.x < 0) {
		return true;
	}

	// 4 �Ѿ��� Sprite ��ü�� ������ ���.
	auto projectile = Sprite::create("projectile.png");
	projectile->setPosition(m_sPlayer->getPosition());
	this->addChild(projectile);

	//���Ϳ� �浹 ���� start
	auto projectileSize = projectile->getContentSize();
	auto physicsBody = PhysicsBody::createCircle(projectileSize.width / 2);
	physicsBody->setDynamic(true);
	physicsBody->setCategoryBitmask((int)PhysicsCategory::Projectile);
	physicsBody->setCollisionBitmask((int)PhysicsCategory::None);
	physicsBody->setContactTestBitmask((int)PhysicsCategory::Monster);

	projectile->setPhysicsBody(physicsBody);
	//���Ϳ� �浹 ���� end

	// 5 offset�� Vec2 ��ü�ν� 
	// normalize()�� ������ x,y��ǥ�� ���� ����ȭ �����ݴϴ�.
	offset.normalize();
	auto shootAmount = offset * 1000;

	// 6 �Ѿ��� ���Ͱ�
	auto realDest = shootAmount + projectile->getPosition();

	// 7 �Ѿ��� �ӵ��� �����ϰ� �߻� �����ݴϴ�.
	// addMonster �κп��� �����Ͽ����� ����.
	auto actionMove = MoveTo::create(2.0f, realDest);
	auto actionRemove = RemoveSelf::create();
	projectile->runAction(Sequence::create(actionMove, actionRemove, nullptr));

	// 8 �ѼҸ�
	SimpleAudioEngine::getInstance()->playEffect(PEW_PEW_SFX);

	return true;
}

bool HelloWorld::onContactBegan(PhysicsContact &contact) {
	//�浹 �� ������ �������� �˴ϴ�.
	auto nodeA = contact.getShapeA()->getBody()->getNode();
	auto nodeB = contact.getShapeB()->getBody()->getNode();

	
	//������ ����� ������ ����.
	// �۾����� �ۿ��� ������ġ�� ��� nullptr�� ��带 ����� �Ǹ�
	// ������ �߻��Ѵ�. �ݵ�� �Ʒ��� if���� �߰��ϵ���.
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
