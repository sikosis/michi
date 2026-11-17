#ifndef MICHI_PROJECT_ITEM_H
#define MICHI_PROJECT_ITEM_H

#include "Project.h"

#include <ListItem.h>

class ProjectItem : public BListItem {
public:
	explicit ProjectItem(const Project& project);
	const Project& Value() const {
		return fProject;
	}
	//---------------------------------------------------------------------------------------------------------------------------------//

	void DrawItem(BView* owner, BRect frame, bool complete) override;
	void Update(BView* owner, const BFont* font) override;

private:
	Project fProject;
};

#endif
