#ifndef GAME_EDITOR_PARTICLE_GENERATOR_H
#define GAME_EDITOR_PARTICLE_GENERATOR_H
#include <game/client/ui.h>

class CParticleGenerator
{
public:
	CParticleGenerator(class CEditor *pEditor);
	~CParticleGenerator();

	void Tick(CUIRect ToolBox, CUIRect ToolBar, CUIRect View);

	bool IsActive() const { return m_Actived; }
	void SetActive(bool State) { m_Actived = State; }

private:
	class CEditor *m_pEditor;
	bool m_Actived;
};

#endif
