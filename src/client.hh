// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __client_hh
#define   __client_hh

/*! @file client.hh
  @brief The Client class maintains the state of a client window by handling
  property changes on the window and some client messages
*/

#include "screen.hh"
#include "otk/strut.hh"
#include "otk/rect.hh"
#include "otk/eventhandler.hh"
#include "otk/ustring.hh"

extern "C" {
#include <X11/Xlib.h>

#ifdef    SHAPE
#include <X11/extensions/shape.h>
#endif // SHAPE
}

#include <string>
#include <list>

namespace ob {

class Frame;
class Screen;

struct Icon {
  unsigned long w, h;
  unsigned long *data;
};

//! The MWM Hints as retrieved from the window property
/*!
  This structure only contains 3 elements, even though the Motif 2.0
  structure contains 5. We only use the first 3, so that is all gets defined.
*/
struct MwmHints {
  unsigned long flags;      //!< A bitmask of Client::MwmFlags values
  unsigned long functions;  //!< A bitmask of Client::MwmFunctions values
  unsigned long decorations;//!< A bitmask of Client::MwmDecorations values
  //! The number of elements in the Client::MwmHints struct
  static const unsigned int elements = 3;
};

//! Maintains the state of a client window.
/*!
  Client maintains the state of a client window. The state consists of the
  hints that the application sets on the window, such as the title, or window
  gravity.
  <p>
  Client also manages client messages for the client window. When the
  application (or any application) requests something to be changed for the
  client, it will call the ActionHandler (for client messages) or update the
  class' member variables and call whatever is nessary to complete the
  change (such as causing a redraw of the titlebar after the title is changed).
*/
class Client : public otk::EventHandler {
public:

  //! The frame window which decorates around the client window
  /*!
    NOTE: This should NEVER be used inside the client class's constructor!
  */
  Frame *frame;

  //! Holds a list of Clients
  typedef std::list<Client*> List;

  //! The possible stacking layers a client window can be a part of
  enum StackLayer {
    Layer_Icon,       //!< 0 - iconified windows, in any order at all
    Layer_Desktop,    //!< 1 - desktop windows
    Layer_Below,      //!< 2 - normal windows w/ below
    Layer_Normal,     //!< 3 - normal windows
    Layer_Above,      //!< 4 - normal windows w/ above
    Layer_Top,        //!< 5 - always-on-top-windows (docks?)
    Layer_Fullscreen, //!< 6 - fullscreeen windows
    Layer_Internal,   //!< 7 - openbox windows/menus
    NUM_LAYERS
  };

  //! Corners of the client window, used for anchor positions
  enum Corner { TopLeft,
                TopRight,
                BottomLeft,
                BottomRight };

  //! Possible window types
  enum WindowType { Type_Desktop, //!< A desktop (bottom-most window)
                    Type_Dock,    //!< A dock bar/panel window
                    Type_Toolbar, //!< A toolbar window, pulled off an app
                    Type_Menu,    //!< An unpinned menu from an app
                    Type_Utility, //!< A small utility window such as a palette
                    Type_Splash,  //!< A splash screen window
                    Type_Dialog,  //!< A dialog window
                    Type_Normal   //!< A normal application window
  };

  //! Possible flags for MWM Hints (defined by Motif 2.0)
  enum MwmFlags { MwmFlag_Functions   = 1 << 0, //!< The MMW Hints define funcs
                  MwmFlag_Decorations = 1 << 1  //!< The MWM Hints define decor
  };

  //! Possible functions for MWM Hints (defined by Motif 2.0)
  enum MwmFunctions { MwmFunc_All      = 1 << 0, //!< All functions
                      MwmFunc_Resize   = 1 << 1, //!< Allow resizing
                      MwmFunc_Move     = 1 << 2, //!< Allow moving
                      MwmFunc_Iconify  = 1 << 3, //!< Allow to be iconfied
                      MwmFunc_Maximize = 1 << 4  //!< Allow to be maximized
                      //MwmFunc_Close    = 1 << 5 //!< Allow to be closed
  };

  //! Possible decorations for MWM Hints (defined by Motif 2.0)
  enum MemDecorations { MwmDecor_All      = 1 << 0, //!< All decorations
                        MwmDecor_Border   = 1 << 1, //!< Show a border
                        MwmDecor_Handle   = 1 << 2, //!< Show a handle (bottom)
                        MwmDecor_Title    = 1 << 3, //!< Show a titlebar
                        //MwmDecor_Menu     = 1 << 4, //!< Show a menu
                        MwmDecor_Iconify  = 1 << 5, //!< Show an iconify button
                        MwmDecor_Maximize = 1 << 6  //!< Show a maximize button
  };

  //! The things the user can do to the client window
  enum Function { Func_Resize     = 1 << 0, //!< Allow resizing
                  Func_Move       = 1 << 1, //!< Allow moving
                  Func_Iconify    = 1 << 2, //!< Allow to be iconified
                  Func_Maximize   = 1 << 3, //!< Allow to be maximized
                  Func_Shade      = 1 << 4, //!< Allow to be shaded
                  Func_Fullscreen = 1 << 5, //!< Allow to be made fullscreen
                  Func_Close      = 1 << 6  //!< Allow to be closed
  };
  //! Holds a bitmask of Client::Function values
  typedef unsigned char FunctionFlags;

  //! The decorations the client window wants to be displayed on it
  enum Decoration { Decor_Titlebar    = 1 << 0, //!< Display a titlebar
                    Decor_Handle      = 1 << 1, //!< Display a handle (bottom)
                    Decor_Border      = 1 << 2, //!< Display a border
                    Decor_Icon        = 1 << 3, //!< Display the window's icon
                    Decor_Iconify     = 1 << 4, //!< Display an iconify button
                    Decor_Maximize    = 1 << 5, //!< Display a maximize button
                    //! Display a button to toggle the window's placement on
                    //! all desktops
                    Decor_AllDesktops = 1 << 6,
                    Decor_Close       = 1 << 7  //!< Display a close button
  };
  //! Holds a bitmask of Client::Decoration values
  typedef unsigned char DecorationFlags;

  //! The event mask to grab on client windows
  static const long event_mask = PropertyChangeMask | FocusChangeMask |
                                 StructureNotifyMask;

  //! The mask of events to not let propogate past the client
  /*!
    This makes things like xprop work on the client window, but means we have
    to explicitly grab clicks that we want.
  */
  static const long no_propagate_mask = ButtonPressMask | ButtonReleaseMask |
                                        ButtonMotionMask;

  //! The number of unmap events to ignore on the window
  int ignore_unmaps;
  
private:
  //! The screen number on which the client resides
  int      _screen;
  
  //! The actual window that this class is wrapping up
  Window   _window;

  //! The id of the group the window belongs to
  Window   _group;

  //! The client which this client is a transient (child) for
  Client *_transient_for;

  //! The clients which are transients (children) of this client
  Client::List _transients;

  //! The desktop on which the window resides (0xffffffff for all desktops)
  unsigned int _desktop;

  //! Normal window title
  otk::ustring  _title;
  //! Window title when iconifiged
  otk::ustring  _icon_title;

  //! The application that created the window
  std::string  _app_name;
  //! The class of the window, can used for grouping
  std::string  _app_class;
  //! The specified role of the window, used for identification
  std::string  _role;

  //! The type of window (what its function is)
  WindowType   _type;

  //! Position and size of the window
  /*!
    This will not always be the actual position of the window on screen, it is
    rather, the position requested by the client, to which the window's gravity
    is applied.
  */
  otk::Rect    _area;

  //! The window's strut
  /*!
    The strut defines areas of the screen that are marked off-bounds for window
    placement. In theory, where this window exists.
  */
  otk::Strut   _strut;

  //! The logical size of the window
  /*!
    The "logical" size of the window is refers to the user's perception of the
    size of the window, and is the value that should be displayed to the user.
    For example, with xterms, this value it the number of characters being
    displayed in the terminal, instead of the number of pixels.
  */
  otk::Size   _logical_size;

  //! Width of the border on the window.
  /*!
    The window manager will set this to 0 while the window is being managed,
    but needs to restore it afterwards, so it is saved here.
  */
  int _border_width;

  //! The minimum aspect ratio the client window can be sized to.
  /*!
    A value of 0 means this is ignored.
  */
  float _min_ratio;
  //! The maximum aspect ratio the client window can be sized to.
  /*!
    A value of 0 means this is ignored.
  */
  float _max_ratio;
  
  //! The minimum size of the client window
  /*!
    If the min is > the max, then the window is not resizable
  */
  otk::Size _min_size;
  //! The maximum size of the client window
  /*!
    If the min is > the max, then the window is not resizable
  */
  otk::Size _max_size;
  //! The size of increments to resize the client window by
  otk::Size _size_inc;
  //! The base size of the client window
  /*!
    This value should be subtracted from the window's actual size when
    displaying its size to the user, or working with its min/max size
  */
  otk::Size _base_size;

  //! Window decoration and functionality hints
  MwmHints _mwmhints;
  
  //! Where to place the decorated window in relation to the undecorated window
  int _gravity;

  //! The state of the window, one of WithdrawnState, IconicState, or
  //! NormalState
  long _wmstate;

  //! True if the client supports the delete_window protocol
  bool _delete_window;
  
  //! Was the window's position requested by the application? if not, we should
  //! place the window ourselves when it first appears
  bool _positioned;
  
  //! Can the window receive input focus?
  bool _can_focus;
  //! Urgency flag
  bool _urgent;
  //! Notify the window when it receives focus?
  bool _focus_notify;
  //! Does the client window have the input focus?
  bool _focused;

  //! The window uses shape extension to be non-rectangular?
  bool _shaped;

  //! The window is modal, so it must be processed before any windows it is
  //! related to can be focused
  bool _modal;
  //! Only the window's titlebar is displayed
  bool _shaded;
  //! The window is iconified
  bool _iconic;
  //! The window is maximized to fill the screen vertically
  bool _max_vert;
  //! The window is maximized to fill the screen horizontally
  bool _max_horz;
  //! The window should not be displayed by pagers
  bool _skip_pager;
  //! The window should not be displayed by taskbars
  bool _skip_taskbar;
  //! The window is a 'fullscreen' window, and should be on top of all others
  bool _fullscreen;
  //! The window should be on top of other windows of the same type
  bool _above;
  //! The window should be underneath other windows of the same type
  bool _below;

  //! The layer in which the window will be stacked, windows in lower layers
  //! are always below windows in higher layers.
  StackLayer _layer;

  //! A bitmask of values in the Client::Decoration enum
  /*!
    The values in the variable are the decorations that the client wants to be
    displayed around it.
  */
  DecorationFlags _decorations;

  //! A bitmask of values in the Client::Decoration enum.
  /*!
    Specifies the decorations that should NOT be displayed on the client.
  */
  DecorationFlags _disabled_decorations;

  //! A bitmask of values in the Client::Function enum
  /*!
    The values in the variable specify the ways in which the user is allowed to
    modify this window.
  */
  FunctionFlags _functions;

  //! Icons for the client as specified on the client window
  Icon *_icons;
  //! The number of icons in _icons
  int _nicons;

  Pixmap _pixmap_icon;
  Pixmap _pixmap_icon_mask;

  //! Retrieves the window's initial gravity
  void getGravity();
  //! Retrieves the desktop hint's value and sets Client::_desktop
  void getDesktop();
  //! Retrieves the window's type and sets Client::_type
  void getType();
  //! Gets the MWM Hints and adjusts Client::_functions and
  //! Client::_decorations
  void getMwmHints();
  //! Gets the position and size of the window and sets Client::_area
  void getArea();
  //! Gets the net_state hint and sets the boolean flags for any states set in
  //! the hint
  void getState();
  //! Determines if the window uses the Shape extension and sets
  //! Client::_shaped
  void getShaped();

  //! Set up what decor should be shown on the window and what functions should
  //! be allowed (Client::_decorations and Client::_functions).
  /*!
    This also updates the NET_WM_ALLOWED_ACTIONS hint.
  */
  void setupDecorAndFunctions();
  
  //! Sets the wm_state to the specified value
  void setWMState(long state);
  //! Adjusts the window's net_state
  /*!
    This should not be called as part of the window mapping process! It is for
    use when updating the state post-mapping.<br>
    Client::applyStartupState is used to do the same things during the mapping
    process.
  */
  void setState(Atom action, long data1, long data2);

  //! Sends the window to the specified desktop
  void setDesktop(unsigned int desktop);
  
  //! Calculates the stacking layer for the client window
  void calcLayer();

  //! Update the protocols that the window supports and adjusts things if they
  //! change
  void updateProtocols();
  //! Updates the WMNormalHints and adjusts things if they change
  void updateNormalHints();
  //! Updates the WMHints and adjusts things if they change
  /*!
    @param initstate Whether to read the initial_state property from the
                     WMHints. This should only be used during the mapping
                     process.
  */
  void updateWMHints(bool initstate = false);
  //! Updates the window's title
  void updateTitle();
  //! Updates the window's icon title
  void updateIconTitle();
  //! Updates the window's application name and class
  void updateClass();
  //! Updates the strut for the client
  void updateStrut();
  //! Updates the window's transient status, and any parents of it
  void updateTransientFor();
  //! Updates the window's icons
  void updateIcons();
  //! Updates the window's kwm icon
  void updateKwmIcon();

  //! Change the client's state hints to match the class' data
  void changeState();
  //! Change the allowed actions set on the client
  void changeAllowedActions();

  //! Request the client to close its window.
  void close();

  //! Shades or unshades the client window
  /*!
    @param shade true if the window should be shaded; false if it should be
                 unshaded.
  */
  void shade(bool shade);

  //! Recursively searches the client 'tree' for a modal client, always skips
  //! the topmost node (the window you're starting with).
  Client *Client::searchModalTree(Client *node, Client *skip);

  //! Recursively searches the client 'tree' for a focused client, always skips
  //! the topmost node (the window you're starting with).
  Client *Client::searchFocusTree(Client *node, Client *skip);

  //! Fires the urgent callbacks which lets the user do what they want with
  //! urgent windows
  void fireUrgent();
  
  //! Fullscreen's or unfullscreen's the client window
  /*!
    @param fs true if the window should be made fullscreen; false if it should
              be returned to normal state.
    @param savearea true to have the client's current size and position saved;
                    otherwise, they are not. You should not save when mapping a
                    new window that is set to fullscreen. This has no effect
                    when restoring a window from fullscreen.
  */
  void fullscreen(bool fs, bool savearea = true);

  //! Iconifies or uniconifies the client window
  /*!
    @param iconic true if the window should be iconified; false if it should be
                  restored.
    @param curdesk If iconic is false, then this determines if the window will
                   be uniconified to the current viewable desktop (true) or to
                   its previous desktop (false)
  */
  void iconify(bool iconic, bool curdesk = true);

  //! Maximize or unmaximize the client window
  /*!
    @param max true if the window should be maximized; false if it should be
               returned to normal size.
    @param dir 0 to set both horz and vert, 1 to set horz, 2 to set vert.
    @param savearea true to have the client's current size and position saved;
                    otherwise, they are not. You should not save when mapping a
                    new window that is set to fullscreen. This has no effect
                    when unmaximizing a window.
  */
  void maximize(bool max, int dir, bool savearea = true);

  //! Internal version of the Client::move function
  /*!
    @param x The X coordinate to move to.
    @param y The Y coordinate to move to.
  */
  void internal_move(int x, int y);
  //! Internal version of the Client::resize function
  /*!
    This also maintains things like the client's minsize, and size increments.
    @param anchor The corner to keep in the same position when resizing.
    @param w The width component of the new size for the client.
    @param h The height component of the new size for the client.
    @param user Specifies whether this is a user-requested change or a
                program requested change.
    @param x An optional X coordinate to which the window will be moved
             after resizing.
    @param y An optional Y coordinate to which the window will be moved
             after resizing.
    The x and y coordinates must both be sepcified together, or they will have
    no effect. When they are specified, the anchor is ignored.
  */
  void internal_resize(Corner anchor, int w, int h,
                       bool user = true, int x = INT_MIN, int y = INT_MIN);

  //! Removes or reapplies the client's border to its window
  /*!
    Used when managing and unmanaging a window.
    @param addborder true if adding the border to the client; false if removing
                     from the client
  */
  void toggleClientBorder(bool addborder);

  //! Applies the states requested when the window mapped
  /*!
    This should be called only once, during the window mapping process. It
    applies things like maximized, and fullscreen.
  */
  void applyStartupState();
  
public:
  //! Constructs a new Client object around a specified window id
  /*!
    @param window The window id that the Client class should handle
    @param screen The screen on which the window resides
  */
  Client(int screen, Window window);
  //! Destroys the Client object
  virtual ~Client();

  //! Returns the screen on which the clien resides
  inline int screen() const { return _screen; }
  
  //! Returns the window id that the Client object is handling
  inline Window window() const { return _window; }

  //! Returns the type of the window, one of the Client::WindowType values
  inline WindowType type() const { return _type; }
  //! Returns if the window should be treated as a normal window.
  /*!
    Some windows (desktops, docks, splash screens) have special rules applied
    to them in a number of places regarding focus or user interaction.
  */
  inline bool normal() const {
    return ! (_type == Type_Desktop || _type == Type_Dock ||
              _type == Type_Splash);
  }
  
  //! Returns the desktop on which the window resides
  /*!
    This value is a 0-based index.<br>
    A value of 0xffffffff indicates that the window exists on all desktops.
  */
  inline unsigned int desktop() const { return _desktop; }
  //! Returns the window's title
  inline const otk::ustring &title() const { return _title; }
  //! Returns the window's title when it is iconified
  inline const otk::ustring &iconTitle() const { return _title; }
  //! Returns the application's name to whom the window belongs
  inline const std::string &appName() const { return _app_name; }
  //! Returns the class of the window
  inline const std::string &appClass() const { return _app_class; }
  //! Returns the program-specified role of the window
  inline const std::string &role() const { return _role; }
  //! Returns if the window can be focused
  /*!
    @return true if the window can receive focus; otherwise, false
  */
  inline bool canFocus() const { return _can_focus; }
  //! Returns if the window has indicated that it needs urgent attention
  inline bool urgent() const { return _urgent; }
  //! Returns if the window wants to be notified when it receives focus
  inline bool focusNotify() const { return _focus_notify; }
  //! Returns if the window is the focused window
  inline bool focused() const { return _focused; }
  //! Returns if the window uses the Shape extension
  inline bool shaped() const { return _shaped; }
  //! Returns the window's gravity
  /*!
    This value determines where to place the decorated window in relation to
    its position without decorations.<br>
    One of: NorthWestGravity, SouthWestGravity, EastGravity, ...,
    SouthGravity, StaticGravity, ForgetGravity
  */
  inline int gravity() const { return _gravity; }
  //! Returns if the application requested the initial position for the window
  /*!
    If the application did not request a position (this function returns false)
    then the window should be placed intelligently by the window manager
    initially
  */
  inline bool positionRequested() const { return _positioned; }
  //! Returns the decorations that the client window wishes to be displayed on
  //! it
  inline DecorationFlags decorations() const { return _decorations; }
  //! Returns the decorations that the user has requested to be disabled on the
  //! client
  inline DecorationFlags disabledDecorations() const
    { return _disabled_decorations; }
  //! Returns the functions that the user can perform on the window
  inline FunctionFlags funtions() const { return _functions; }

  //! Return the client this window is transient for
  inline Client *transientFor() const { return _transient_for; }

  //! Returns if the window is modal
  /*!
    If the window is modal, then no other windows that it is related to can get
    focus while it exists/remains modal.
  */
  inline bool modal() const { return _modal; }
  //! The window should not be displayed by pagers
  inline bool skipPager() const { return _skip_pager; }
  //! The window should not be displayed by taskbars
  inline bool skipTaskbar() const { return _skip_taskbar; } 
  //! Returns if the window is shaded
  /*!
    When the window is shaded, only its titlebar is visible.
  */
  inline bool shaded() const { return _shaded; }
  //! Returns if the window is in fullscreen mode
  inline bool fullscreen() const { return _fullscreen; }
  //! Returns if the window is iconified
  /*!
    When the window is iconified, it is not visible at all (except in iconbars/
    panels/etc that want to show lists of iconified windows
  */
  inline bool iconic() const { return _iconic; }
  //! Returns if the window is maximized vertically
  inline bool maxVert() const { return _max_vert; }
  //! Returns if the window is maximized horizontally
  inline bool maxHorz() const { return _max_horz; }
  //! Returns the window's stacking layer
  inline StackLayer layer() const { return _layer; }

  //! Returns the logical size of the window
  /*!
    The "logical" size of the window is refers to the user's perception of the
    size of the window, and is the value that should be displayed to the user.
    For example, with xterms, this value it the number of characters being
    displayed in the terminal, instead of the number of pixels.
  */
  const otk::Size &logicalSize() const { return _logical_size; }

  //! Returns the position and size of the client relative to the root window
  /*!
    Note that this value is *not* the size and position of the window's
    frame, though the position will often line up.<br>
    If you want the frame's area, use Frame::area() instead.
  */
  inline const otk::Rect &area() const { return _area; }

  //! Returns the client's strut definition
  inline const otk::Strut &strut() const { return _strut; }

  //! Returns an icon for the window
  /*!
    The icon chosen will be the smallest icon available that is still bigger or
    equal to the specified Size.<br>
    If none that meet the requirements is found, the largest icon that is
    smaller than the specified size will be returned.
  */
  const Icon *icon(const otk::Size &s) const;

  //! Returns the pixmap for the pixmap icon specified on the window (or None)
  /*!
    The icon given by Client::icon should take precedence over this icon/mask.
  */
  Pixmap pixmapIcon() const { return _pixmap_icon; }
  //! Returns the mask for the pixmap icon specified on the window (or None)
  /*!
    The icon given by Client::icon should take precedence over this icon/mask.
  */
  Pixmap pixmapIconMask() const { return _pixmap_icon_mask; }
  
  //! Move the window (actually, its frame) to a position.
  /*!
    This moves the window so that the top-left corner of its frame will be at
    the position specified.
    @param x The X coordinate to move to.
    @param y The Y coordinate to move to.
  */
  void move(int x, int y);
  
  //! Resizes the client window, anchoring it in a given corner
  /*!
    This also maintains things like the client's minsize, and size increments.
    @param anchor The corner to keep in the same position when resizing.
    @param w The width component of the new size for the client.
    @param h The height component of the new size for the client.
  */
  void resize(Corner anchor, int w, int h);

  //! Reapplies the maximized state to the window
  /*!
    Use this to make the window readjust its maximized size to new
    surroundings (struts, etc).
  */
  void remaximize();

  //! Shows the window if it should be shown, or hides it
  /*!
    Used when changing desktops, the window's state, etc.
  */
  void showhide();
  
  //! Choose a mask of decorations to not display on the client
  /*!
    Pass a value of 0 to the function to turn all decorations back on. Note
    that you cannot add decorations to a window with this, you can only remove
    decorations that would otherwise have been displayed.
    @param flags The mask of values from Client::Decoration to specify which
                 decorations should not be displayed.
  */
  void disableDecorations(DecorationFlags flags);
  
  //! Return a modal child of the client window
  /*!
    @return A modal child of the client window, or 0 if none was found.
  */
  Client *findModalChild();

  //! Attempt to focus the client window
  bool focus();

  //! Remove focus from the client window
  void unfocus() const;

  //! Validate client, by making sure no Destroy or Unmap events exist in
  //! the event queue for the window.
  /*!
    @return true if the client is valid; false if the client has already
            been unmapped/destroyed, and so is invalid.
  */
  bool validate() const;

  void installColormap(bool install) const;
  
  virtual void focusHandler(const XFocusChangeEvent &e);
  virtual void unfocusHandler(const XFocusChangeEvent &e);
  virtual void propertyHandler(const XPropertyEvent &e);
  virtual void clientMessageHandler(const XClientMessageEvent &e);
  virtual void configureRequestHandler(const XConfigureRequestEvent &e);
  virtual void unmapHandler(const XUnmapEvent &e);
  virtual void destroyHandler(const XDestroyWindowEvent &e);
  virtual void reparentHandler(const XReparentEvent &e);
  virtual void mapRequestHandler(const XMapRequestEvent &e);
#if defined(SHAPE)
  virtual void shapeHandler(const XShapeEvent &e);
#endif // SHAPE

  friend void Screen::manageWindow(Window);
  friend void Screen::unmanageWindow(Client *);
};

}

#endif // __client_hh
