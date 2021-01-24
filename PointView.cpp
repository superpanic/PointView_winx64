#include <iostream>
#include "IllustratorSDK.h"
#include "AIAnnotator.h"
#include "AIAnnotatorDrawer.h"

// Tell Xcode to export the following symbols
#if defined(__GNUC__)
#pragma GCC visibility push(default)
#endif

// Plug-in entry point
extern "C" ASAPI ASErr PluginMain(char * caller, char* selector, void* message);

// Tell Xcode to return to default visibility for symbols
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

extern "C" {
	SPBlocksSuite *sSPBlocks = NULL; // memory handling
	AIUnicodeStringSuite *sAIUnicodeString = NULL; // unicode string helper
	AIUserSuite *sAIUser = NULL; // alerts
	AIAnnotatorSuite *sAIAnnotator = NULL; // register plug-in to recieve annotation update messages
	AIAnnotatorDrawerSuite *sAIAnnotatorDrawer = NULL; // draw annotations
	AIMatchingArtSuite *sAIMatchingArt = NULL; // get selected art objects
	AIMdMemorySuite *sAIMemory = NULL; // handling memory
	AIArtSuite *sAIArt = NULL; // art objects
	AIPathSuite *sAIPath = NULL; // working with paths
	AIDocumentViewSuite *sAIDocumentView = NULL; // converting art to view coordinates
	AILayerSuite *sAILayer = NULL; // to access current layer color
	AINotifierSuite *sAINotifier = NULL; // register move art notification
}

typedef struct {
	AIAnnotatorHandle annotatorHandle;
	//AINotifierHandle notifierHandle;
} Globals;

Globals *g = nullptr;

static ai::int32 POINT_VIEW_RADIUS = 6;
static AIReal POINT_VIEW_LINE_WIDTH = 2.0;

static AIErr StartupPlugin(SPInterfaceMessage *message);
static AIErr ShutdownPlugin(SPInterfaceMessage *message);
static AIErr DrawAnnotation(void *message);
static void PointView_SetAIRectSize(AIRect *r, AIPoint *p, ai::int32 radius);



extern "C" ASAPI ASErr PluginMain(char *caller, char *selector, void *message) {

	ASErr error = kNoErr;
	SPPlugin *self = ((SPMessageData *)message)->self;
	SPBasicSuite* sSPBasic = ((SPMessageData*)message)->basic;

	if (sSPBasic->IsEqual(caller, kSPInterfaceCaller)) {

		error = sSPBasic->AcquireSuite(kSPBlocksSuite, kSPBlocksSuiteVersion, (const void**)&sSPBlocks);
		error = sSPBasic->AcquireSuite(kAIUserSuite, kAIUserSuiteVersion, (const void **)&sAIUser);
		error = sSPBasic->AcquireSuite(kAIUnicodeStringSuite, kAIUnicodeStringSuiteVersion, (const void **)&sAIUnicodeString);

		if (sSPBasic->IsEqual(selector, kSPInterfaceStartupSelector)) {
			// sAIUser->MessageAlert(ai::UnicodeString("PointView Loaded"));
			StartupPlugin((SPInterfaceMessage *)message);
		} else if (sSPBasic->IsEqual(selector, kSPInterfaceShutdownSelector)) {
			// sAIUser->MessageAlert(ai::UnicodeString("Goodbye!"));
			ShutdownPlugin((SPInterfaceMessage *)message);
		}
		
		// error = sSPBasic->AcquireSuite(kAIUnicodeStringSuite, kAIUnicodeStringSuiteVersion, (const void **)&sAIUnicodeString);
		error = sSPBasic->ReleaseSuite(kAIUnicodeStringSuite, kAIUnicodeStringSuiteVersion);
		// error = sSPBasic->AcquireSuite(kAIUserSuite, kAIUserSuiteVersion, (const void **)&sAIUser);
		error = sSPBasic->ReleaseSuite(kAIUserSuite, kAIUserSuiteVersion);
		// error = sSPBasic->AcquireSuite(kSPBlocksSuite, kSPBlocksSuiteVersion, (const void**)&sSPBlocks);
		error = sSPBasic->ReleaseSuite(kSPBlocksSuite, kSPBlocksSuiteVersion);

	} 
	
	/*
	else if (sSPBasic->IsEqual(caller, kCallerAINotify)) {
		if (sSPBasic->IsEqual(selector, kSelectorAINotify)) {
			// draging art object does not trigger this notification
			std::cout << "### selection changed ###";
		}
	}
	*/

	else if (sSPBasic->IsEqual(caller, kCallerAIAnnotation)) {


		if (sSPBasic->IsEqual(selector, kSelectorAIDrawAnnotation)) {
			error = DrawAnnotation((AIAnnotatorMessage *)message);
			
		} else if (sSPBasic->IsEqual(selector, kSelectorAIInvalAnnotation)) {
			error = sSPBasic->AcquireSuite(kAIDocumentViewSuite, kAIDocumentViewVersion, (const void **)&sAIDocumentView);
			error = sSPBasic->AcquireSuite(kAIAnnotatorDrawerSuite, kAIAnnotatorDrawerVersion, (const void **)&sAIAnnotatorDrawer);
			
			AIRealRect updateRect;
			AIRect portBounds;
			
			error = sAIDocumentView->GetDocumentViewInvalidRect(NULL, &updateRect);
			portBounds.left = _AIRealRoundToShort(updateRect.left) - 1;
			portBounds.top = _AIRealRoundToShort(updateRect.top) + 1;
			portBounds.right = _AIRealRoundToShort(updateRect.right) + 1;
			portBounds.bottom = _AIRealRoundToShort(updateRect.bottom) - 1;

			sAIAnnotator->InvalAnnotationRect(NULL, &portBounds);

			// error = sSPBasic->AcquireSuite(kAIDocumentViewSuite, kAIDocumentViewVersion, (const void **)&sAIDocumentView);
			error = sSPBasic->ReleaseSuite(kAIDocumentViewSuite, kAIDocumentViewVersion);
			// error = sSPBasic->AcquireSuite(kAIAnnotatorDrawerSuite, kAIAnnotatorDrawerVersion, (const void **)&sAIAnnotatorDrawer);
			error = sSPBasic->ReleaseSuite(kAIAnnotatorDrawerSuite, kAIAnnotatorDrawerVersion);
		}

	}

	return error;
}

static AIErr StartupPlugin(SPInterfaceMessage *message) {
	ASErr error = kNoErr;
	SPBasicSuite* sSPBasic = message->d.basic;

	error = sSPBasic->AllocateBlock(sizeof(Globals), (void **)&g);
	if (!error) {
		message->d.globals = g;
	}

	/*
	// register "selection changed" notification
	sSPBasic->AcquireSuite(kAINotifierSuite, kAINotifierVersion, (const void **)&sAINotifier);
	error = sAINotifier->AddNotifier(message->d.self, "PointView selection changed notifier", kAIArtSelectionChangedNotifier, &(g->notifierHandle));
	sSPBasic->ReleaseSuite(kAINotifierSuite, kAINotifierVersion);
	*/

	// register annotator
	sSPBasic->AcquireSuite(kAIAnnotatorSuite, kAIAnnotatorVersion, (const void **)&sAIAnnotator);
	error = sAIAnnotator->AddAnnotator(message->d.self, "PointView Annotator", &(g->annotatorHandle));
	sSPBasic->ReleaseSuite(kAIAnnotatorSuite, kAIAnnotatorVersion);

	return error;
}

static AIErr ShutdownPlugin(SPInterfaceMessage *message) {
	ASErr error = kNoErr;
	if (g != nullptr) {
		message->d.basic->FreeBlock(g);
		g = nullptr;
		message->d.globals = nullptr;
	}
	return error;
}

static AIErr DrawAnnotation(void *message) {
	AIErr error = kNoErr;

	SPBasicSuite* sSPBasic = ((SPMessageData*)message)->basic;

	AIAnnotatorMessage *annotatorMessage = (AIAnnotatorMessage *)message;
	AIAnnotatorDrawer *annotatorDrawer = (AIAnnotatorDrawer *)annotatorMessage->drawer;

	error = sSPBasic->AcquireSuite(kAIMatchingArtSuite, kAIMatchingArtVersion, (const void **)&sAIMatchingArt);
	error = sSPBasic->AcquireSuite(kAIMdMemorySuite, kAIMdMemoryVersion, (const void **)&sAIMemory);
	error = sSPBasic->AcquireSuite(kAIArtSuite, kAIArtVersion, (const void **)&sAIArt);
	error = sSPBasic->AcquireSuite(kAIPathSuite, kAIPathVersion, (const void **)&sAIPath);
	error = sSPBasic->AcquireSuite(kAIDocumentViewSuite, kAIDocumentViewVersion, (const void **)&sAIDocumentView);
	error = sSPBasic->AcquireSuite(kAIAnnotatorDrawerSuite, kAIAnnotatorDrawerVersion, (const void **)&sAIAnnotatorDrawer);
	error = sSPBasic->AcquireSuite(kAILayerSuite, kAILayerVersion, (const void **)&sAILayer);

	if (sAIMatchingArt->IsSomeArtSelected()) {

		AIArtHandle **selectedArtHandle;
		ai::int32 selectedArtCount;
		// selectedArtHandle is allocated
		sAIMatchingArt->GetSelectedArt(&selectedArtHandle, &selectedArtCount);

		AIArtHandle art;

		for (int i = 0; i < selectedArtCount; i++) {
			art = (*selectedArtHandle)[i];
			short artType;
			sAIArt->GetArtType(art, &artType);

			if (artType == kPathArt) {
				AIBoolean closed;
				sAIPath->GetPathClosed(art, &closed);
				if (!closed) {
					ai::int16 segmentCount;
					sAIPath->GetPathSegmentCount(art, &segmentCount);
					if (segmentCount > 1) {

						AIPathSegment firstSegment;
						AIPathSegment lastSegment;

						sAIPath->GetPathSegments(art, 0, 1, &firstSegment);
						sAIPath->GetPathSegments(art, segmentCount - 1, 1, &lastSegment);

						AIPoint startPointView;
						AIPoint endPointView;

						error = sAIDocumentView->ArtworkPointToViewPoint(NULL, &(firstSegment.p), &startPointView);
						error = sAIDocumentView->ArtworkPointToViewPoint(NULL, &(lastSegment.p), &endPointView);

						AIRect startRect;
						AIRect endRect;

						PointView_SetAIRectSize(&startRect, &startPointView, POINT_VIEW_RADIUS);
						PointView_SetAIRectSize(&endRect, &endPointView, POINT_VIEW_RADIUS);

						AILayerHandle layer;
						sAIArt->GetLayerOfArt(art, &layer);

						AIRGBColor col;
						sAILayer->GetLayerColor(layer, &col);

						sAIAnnotatorDrawer->SetColor(annotatorDrawer, col);
						sAIAnnotatorDrawer->SetLineWidth(annotatorDrawer, POINT_VIEW_LINE_WIDTH);

						sAIAnnotatorDrawer->DrawEllipse(annotatorDrawer, startRect, false);
						sAIAnnotatorDrawer->DrawEllipse(annotatorDrawer, endRect, false);

					}
				}
			}


		}

		// free selected art handle
		sAIMemory->MdMemoryDisposeHandle((AIMdMemoryHandle)selectedArtHandle);
	}

	// error = sSPBasic->AcquireSuite(kAIMatchingArtSuite, kAIMatchingArtVersion, (const void **)&sAIMatchingArt);
	error = sSPBasic->ReleaseSuite(kAIMatchingArtSuite, kAIMatchingArtVersion);
	// error = sSPBasic->AcquireSuite(kAIMdMemorySuite, kAIMdMemoryVersion, (const void **)&sAIMemory);
	error = sSPBasic->ReleaseSuite(kAIMdMemorySuite, kAIMdMemoryVersion);
	// error = sSPBasic->AcquireSuite(kAIArtSuite, kAIArtVersion, (const void **)&sAIArt);
	error = sSPBasic->ReleaseSuite(kAIArtSuite, kAIArtVersion);
	// error = sSPBasic->AcquireSuite(kAIPathSuite, kAIPathVersion, (const void **)&sAIPath);
	error = sSPBasic->ReleaseSuite(kAIPathSuite, kAIPathVersion);
	// error = sSPBasic->AcquireSuite(kAIDocumentViewSuite, kAIDocumentViewVersion, (const void **)&sAIDocumentView);
	error = sSPBasic->ReleaseSuite(kAIDocumentViewSuite, kAIDocumentViewVersion);
	// error = sSPBasic->AcquireSuite(kAIAnnotatorDrawerSuite, kAIAnnotatorDrawerVersion, (const void **)&sAIAnnotatorDrawer);
	error = sSPBasic->ReleaseSuite(kAIAnnotatorDrawerSuite, kAIAnnotatorDrawerVersion);
	// error = sSPBasic->AcquireSuite(kAILayerSuite, kAILayerVersion, (const void **)&sAILayer);
	error = sSPBasic->ReleaseSuite(kAILayerSuite, kAILayerVersion);

	return error;
}

static void PointView_SetAIRectSize(AIRect *r, AIPoint *p, ai::int32 radius) {
	r->left = p->h - radius;
	r->right = p->h + radius;
	r->top = p->v - radius;
	r->bottom = p->v + radius;
}