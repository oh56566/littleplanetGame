#include "InteractPromptWidget.h"

void UInteractPromptWidget::SetPrompt(const FText& Text)
{
	const bool bWantVisible = !Text.IsEmpty();
	const bool bTextChanged = !CurrentPrompt.EqualTo(Text);
	const bool bVisChanged  = bWantVisible != bIsVisible;

	if (!bTextChanged && !bVisChanged)
	{
		return;
	}

	CurrentPrompt = Text;
	bIsVisible    = bWantVisible;

	SetVisibility(bIsVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	OnPromptUpdated(CurrentPrompt, bIsVisible);
}
