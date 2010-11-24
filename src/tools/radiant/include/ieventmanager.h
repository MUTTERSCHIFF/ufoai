#ifndef IEVENTMANAGER_H_
#define IEVENTMANAGER_H_

#include <list>
#include <map>
#include <string>

#include "iselection.h"

#include "generic/constant.h"
#include "generic/callbackfwd.h"

// GTK forward declaration
typedef struct _GtkObject GtkObject;
typedef struct _GtkWindow GtkWindow;
typedef struct _GtkWidget GtkWidget;
typedef struct _GdkEventButton GdkEventButton;
typedef struct _GdkEventKey GdkEventKey;

/* greebo: Below are the actual events that are "read" by the views/observers to
 * interpret the mouseclicks. */

namespace ui {

	// The possible modes when in "component manipulation mode"
	enum XYViewEvent {
		xyNothing,		// unrecognised event
		xyMoveView,		// drag the view around
		xySelect,		// selection / clip
		xyZoom,			// drag-zoom operator
		xyCameraMove,	// the button used to drag the camera around
		xyCameraAngle,	// the button used to change camera angle
		xyNewBrushDrag	// used to create new brushes
	};

	// These are the buttons for the camera view
	enum CamViewEvent {
		camNothing,				// nothing special, event can be passed to the windowobservers
		camEnableFreeLookMode,	// used to enable the free look mode in the camera view
		camDisableFreeLookMode	// used to disable the free look mode in the camera view
	};

	// If the click is passed to the windowobservers, these are the possibilites
	enum ObserverEvent {
		obsNothing,		// any uninterpreted/unsupported combination
		obsManipulate,	// manipulate an object by drag or click
		obsSelect,		// selection toggle
		obsToggle,		// selection toggle
		obsToggleFace,	// selection toggle (face)
		obsReplace,		// replace/cycle selection through possible candidates
		obsReplaceFace,	// replace/cycle selection through possible face candidates
		obsCopyTexture,	// copy texture from object
		obsPasteTexture,	// paste texture to object (projected)
		obsPasteTextureToBrush // paste texture to all brush faces of the selected brush
	};
} // namespace ui


class IEvent
{
public:
	virtual ~IEvent() {}

	// Handles the incoming keyUp / keyDown calls
	virtual void keyUp() = 0;
	virtual void keyDown() = 0;

	// Returns true if this event could be toggled (returns false if the event is not a Toggle).
	virtual bool setToggled(const bool toggled) = 0;

	/** greebo: Returns true if the event is a Toggle (or a subtype of a Toggle)
	 */
	virtual bool isToggle() const = 0;

	// Enables/disables this event
	virtual void setEnabled(const bool enabled) = 0;

	// Connect a GtkWidget to this event (the event must support the according widget).
	virtual void connectWidget(GtkWidget* widget) = 0;

	// Exports the current state to the widgets
	virtual void updateWidgets() = 0;
};

/* greebo: The mouse event manager provides methods to interpret mouse clicks.
 */
class IMouseEvents {
public:
	virtual ~IMouseEvents() {}

	// Return the ObserverEvent type for a given GdkEventButton
	virtual ui::CamViewEvent getCameraViewEvent(GdkEventButton* event) = 0;

	// Return the ObserverEvent type for a given GdkEventButton
	virtual ui::ObserverEvent getObserverEvent(GdkEventButton* event) = 0;
	virtual ui::ObserverEvent getObserverEvent(const unsigned int& state) = 0;

	// Return the current XYView event for a GdkEventMotion state or an GdkEventButton
	virtual ui::XYViewEvent getXYViewEvent(GdkEventButton* event) = 0;
	virtual ui::XYViewEvent getXYViewEvent(const unsigned int& state) = 0;

	virtual bool stateMatchesXYViewEvent(const ui::XYViewEvent& xyViewEvent, GdkEventButton* event) = 0;
	virtual bool stateMatchesXYViewEvent(const ui::XYViewEvent& xyViewEvent, const unsigned int& state) = 0;

	virtual bool stateMatchesObserverEvent(const ui::ObserverEvent& observerEvent, GdkEventButton* event) = 0;

	virtual bool stateMatchesCameraViewEvent(const ui::CamViewEvent& camViewEvent, GdkEventButton* event) = 0;

	virtual std::string printXYViewEvent(const ui::XYViewEvent& xyViewEvent) = 0;
	virtual std::string printObserverEvent(const ui::ObserverEvent& observerEvent) = 0;

	virtual float getCameraStrafeSpeed() = 0;
	virtual float getCameraForwardStrafeFactor() = 0;
	virtual bool strafeActive(unsigned int& state) = 0;
	virtual bool strafeForwardActive(unsigned int& state) = 0;
};

class IAccelerator
{
public:
	virtual ~IAccelerator() {}

	// Get/set the GDK key value
	virtual void setKey(const unsigned int& key) = 0;
	virtual unsigned int getKey() const = 0;

	// Get/Set the modifier flags
	virtual void setModifiers(const unsigned int& modifiers) = 0;
	virtual unsigned int getModifiers() const = 0;

	// Connect this IEvent to this accelerator
	virtual void connectEvent(IEvent* event) = 0;
};

// Event visitor class
class IEventVisitor {
public:
	virtual ~IEventVisitor() {}

	virtual void visit(const std::string& eventName, const IEvent* event) = 0;
};

class IEventManager
{
public:
	INTEGER_CONSTANT(Version, 1);
	STRING_CONSTANT(Name, "EventManager");

	virtual ~IEventManager() {}

	/* Create an accelerator using the given arguments and add it to the list
	 *
	 * @key: The symbolic name of the key, e.g. "A", "Esc"
	 * @modifierStr: A string containing the modifiers, e.g. "Shift+Control" or "Shift"
	 *
	 * @returns: the pointer to the newly created accelerator object */
	virtual IAccelerator* addAccelerator(const std::string& key, const std::string& modifierStr) = 0;
	// The same as above, but with GDK event values as argument (event->keyval, event->state)
	virtual IAccelerator* addAccelerator(GdkEventKey* event) = 0;
	virtual IAccelerator* findAccelerator(const IEvent* event) = 0;

	/* greebo: This is to avoid cyclic dependencies, because the eventmanager depends
	 * on the selectionsystem, the selectionsystem on the gridmodule, the gridmodule on
	 * the eventmanager, and there we have our cycle. Call this before any mouse events
	 * have to be interpreted!
	 */
	virtual void connectSelectionSystem(SelectionSystem* selectionSystem) = 0;

	/* greebo: Returns the mouse event "manager" providing a separate interface for
	 * handling mouse events. I moved this into a separate interface to keep
	 * the IEventManager interface cleaner.
	 */
	virtual IMouseEvents& MouseEvents() = 0;

	// Creates a new command that calls the given callback when invoked
	virtual IEvent* addCommand(const std::string& name, const Callback& callback) = 0;

	// Creates a new keyevent that calls the given callback when invoked
	virtual IEvent* addKeyEvent(const std::string& name, const Callback& keyUpCallback, const Callback& keyDownCallback) = 0;

	// Creates a new toggle event that calls the given callback when toggled
	virtual IEvent* addToggle(const std::string& name, const Callback& onToggled) = 0;
	virtual void setToggled(const std::string& name, const bool toggled) = 0;

	virtual IEvent* addWidgetToggle(const std::string& name) = 0;
	virtual IEvent* addRegistryToggle(const std::string& name, const std::string& registryKey) = 0;

	// Returns the pointer to the command specified by the <given> commandName
	virtual IEvent* findEvent(const std::string& name) = 0;
	virtual IEvent* findEvent(GdkEventKey* event) = 0;

	// Retrieves the event name for the given IEvent
	virtual std::string getEventName(IEvent* event) = 0;

	// Connects the given accelerator to the given command (identified by the string)
	virtual void connectAccelerator(IAccelerator* accelerator, const std::string& command) = 0;
	// Disconnects the given command from any accelerators
	virtual void disconnectAccelerator(const std::string& command) = 0;

	// Connects/disconnects the keyboard handlers of the keyeventmanager to the specified window, so that key events are catched
	virtual void connect(GtkObject* object) = 0;
	virtual void disconnect(GtkObject* object) = 0;

	// Connects/Disconnects a Dialog Window to the eventmanager. Dialog windows get the chance
	// to process an incoming keypress event, BEFORE the global shortcuts are searched and launched.
	virtual void connectDialogWindow(GtkWindow* window) = 0;
	virtual void disconnectDialogWindow(GtkWindow* window) = 0;

	// Tells the key event manager about the main window so that the accelgroup can be connected correctly
	virtual void connectAccelGroup(GtkWindow* window) = 0;

	// Loads the shortcut->command associations from the XMLRegistry
	virtual void loadAccelerators() = 0;

	// Enables/Disables the specified command
	virtual void enableEvent(const std::string& eventName) = 0;
	virtual void disableEvent(const std::string& eventName) = 0;

	// Removes the given event and disconnects all accelerators from it
	virtual void removeEvent(const std::string& eventName) = 0;

	// Visit each event with the given class
	virtual void foreachEvent(IEventVisitor* eventVisitor) = 0;

	/* greebo: Retrieves a string representation of the modifiers set in <modifierFlags>
	 * (This is used internally by Modifers class)
	 *
	 * @forMenu:
	 * <true> yields a string of type: Ctrl-Shift
	 * <false> results in a string of type: CTRL+SHIFT
	 */
	virtual std::string getModifierStr(const unsigned int& modifierFlags, bool forMenu = false) = 0;

	/* greebo: Retrieves the string representation of the given GDK <event>
	 */
	virtual std::string getGDKEventStr(GdkEventKey* event) = 0;

	virtual std::string getAcceleratorStr(const IEvent* event, bool forMenu) = 0;

	/** greebo: Returns the current keyboard eventkey state
	 */
	virtual unsigned int getModifierState() = 0;
};

// Module definitions

#include "modulesystem.h"

template<typename Type>
class GlobalModule;
typedef GlobalModule<IEventManager> GlobalEventManagerModule;

template<typename Type>
class GlobalModuleRef;
typedef GlobalModuleRef<IEventManager> GlobalEventManagerModuleRef;

// This is the accessor for the event manager
inline IEventManager& GlobalEventManager() {
	return GlobalEventManagerModule::getTable();
}

#endif /*IEVENTMANAGER_H_*/
