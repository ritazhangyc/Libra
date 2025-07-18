#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"

class Game;

struct Turret
{
public:
	Vec2			m_faceDirection;
	float			m_orientationGoal;
	float			m_localOrientationDegree;
	float			m_worldOrientationDegree;
};


class Player : public Entity
{

public:
	Player(Map* mapInstance, Vec2 const& spawnPosition, float spawnDegrees);
	~Player();

	virtual void			StartUp() override;
	void					Update(float deltaSecond) override; 
	void					Render() override;
 	void					Die() override;
	void					HitByBullet(Entity* bullet) override;
 	void					GetDamage(int damageValue) override;
	virtual IntVec2 		GetTilePosition() const override;
	
	void					SpawnTurret();
	void					RunFrame(float deltaSecond);
	void					BeginFrame();
	void					UpdateInputKeyboard(float deltaSecond);
	void					UpdateInputController(float deltaSecond);
	void					UpdatePlayerRotation(float deltaSecond, float target);
	void					UpdatePlayerPosition(float deltaSecond, float proportion);
	void					UpdateTurretRotation(float deltaSecond, float target);
	void					ShootBullet(float deltaSecond);
	void					EndFrame();

	void					RenderTank() const;
	void					RenderTurret() const;
	void					RespawnPlayer();
	void					InvincibleModeOrNot();

	Turret* const			GetTurret() const;
	float const				GetTargetDegree() const;

private:
	Turret*					m_turret;
	float					m_targetDegree;
	float					m_shootInterval;
	bool					m_canShoot = true;
	bool					m_invincible = false;
};

