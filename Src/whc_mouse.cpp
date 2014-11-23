#include "whc_mouse.h"

/*Initialisation des variables de classe*/

//Buttons
//float whc_button::c_milliseconds = 0; // Default Initialisation of  counter for milliseconds
float whc_button::c_gapSecond = 0.25; // Default Initialisation of  max time between two clics to be a double clic
whc_button::whc_eventclic whc_button::c_mouseLastClic = {none,clock()} ; //{none,whc_button::c_milliseconds}; //

std::vector<whc_button::whc_eventclic> whc_button::c_mouseClicHistory;

//Wheels
int whc_wheel::c_mouse_z_prev = 0;
int whc_wheel::c_mouse_w_prev = 0;

//
whc_button::whc_button() : m_Down(false), m_Double(false), m_DragDrop(false), m_Processed(true)
{
	//ctor
	m_dragpos.x = 0; m_dragpos.y = 0;
	whc_button::c_mouseClicHistory.clear();
	whc_button::c_mouseClicHistory.push_back (whc_button::c_mouseLastClic);
}

whc_button::~whc_button()
{
	//dtor
}

bool whc_button::c_IsThisADoubleClic (whc_mousebutton button)
{
	bool result = false ;
	whc_button::c_mouseLastClic = {button,clock()} ;//{button,whc_button::c_milliseconds}; //

	if (whc_button::c_mouseClicHistory.size()>0)
	{
		if (whc_button::c_mouseClicHistory[0].button == whc_button::c_mouseLastClic.button)
		{

			clock_t present  = whc_button::c_mouseLastClic.cpu_clock_ticks ; 			// present  down
			clock_t past     = whc_button::c_mouseClicHistory[0].cpu_clock_ticks ; 	// previous down
			/*
			float present  = whc_button::c_mouseLastClic.cpu_clock_ticks ; 			// present  down
			float past     = whc_button::c_mouseClicHistory[0].cpu_clock_ticks ; 	// previous down
			*/

			float nbr_ticks_per_sec = CLOCKS_PER_SEC * 1.0 ;
			//float nbr_ticks_per_sec = 0.01 ;
			clock_t gap_in_ticks = (present - past) ;
			float gap_in_sec = gap_in_ticks / nbr_ticks_per_sec ;

			if (gap_in_sec < whc_button::c_gapSecond)
			{
				result = true ;
			}
		}
	}

	whc_button::c_mouseClicHistory.clear();
	whc_button::c_mouseClicHistory.push_back (whc_button::c_mouseLastClic);

	return result ;
}

void whc_button::c_CollectEvent (int& mouseSignal, volatile int& mouse_x, volatile int& mouse_y, whc_button& mouseClicLeft, whc_button& mouseClicMiddle, whc_button& mouseClicRight)
{
    switch(mouseSignal)
    {
    	case MOUSE_FLAG_LEFT_DOWN  :
    		mouseClicLeft.SetDown();
			mouseClicLeft.SetToBeProcessed();
			mouseClicLeft.SetDouble(c_IsThisADoubleClic(left));
			mouseClicLeft.SetDragDrop(false);
			mouseClicLeft.SetDragPos(mouse_x,mouse_y);
			break;

		case MOUSE_FLAG_LEFT_UP    :
			mouseClicLeft.SetUp();
			mouseClicLeft.SetDragDrop((mouseClicLeft.dragPos().x == mouse_x) && (mouseClicLeft.dragPos().y == mouse_y));
			break;

		case MOUSE_FLAG_MIDDLE_DOWN :
    		mouseClicMiddle.SetDown();
			mouseClicMiddle.SetToBeProcessed();
			mouseClicMiddle.SetDouble(c_IsThisADoubleClic(middle));
			mouseClicMiddle.SetDragDrop(false);
			mouseClicMiddle.SetDragPos(mouse_x, mouse_y);
			break;

		case MOUSE_FLAG_MIDDLE_UP   :
			mouseClicMiddle.SetUp();
			mouseClicMiddle.SetDragDrop((mouseClicMiddle.dragPos().x == mouse_x) && (mouseClicMiddle.dragPos().y == mouse_y));
			break;

		case MOUSE_FLAG_RIGHT_DOWN :
    		mouseClicRight.SetDown();
			mouseClicRight.SetToBeProcessed();
			mouseClicRight.SetDouble(c_IsThisADoubleClic(right));
			mouseClicRight.SetDragDrop(false);
			mouseClicRight.SetDragPos(mouse_x, mouse_z);
			break;

		case MOUSE_FLAG_RIGHT_UP   :
			mouseClicRight.SetUp();
			mouseClicRight.SetDragDrop((mouseClicRight.dragPos().x == mouse_x) && (mouseClicRight.dragPos().y == mouse_y));
			break;

		default :
			break;
    }
}


//------------------------------------------------------------------------------------------------------------------------------


whc_wheel::whc_wheel() : m_level(0), m_gain(0), m_yield(0), m_speed(0), m_Processed(true)
{
	//ctor
}

whc_wheel::~whc_wheel()
{
	//dtor
}

void whc_wheel::c_Init (volatile int& mouse_z, volatile int& mouse_w)
{
	whc_wheel::c_mouse_z_prev = mouse_z ;
	whc_wheel::c_mouse_w_prev = mouse_w ;
}

void whc_wheel::c_CollectEvent (int& mouseSignal, volatile int& mouse_z, volatile int& mouse_w, whc_wheel& mouseScroll , whc_wheel& mouseRoll)
{
	//Mouse wheel event
    switch(mouseSignal)
    {
    	case MOUSE_FLAG_MOVE_Z  :   //Scroll
    		if (key[KEY_LSHIFT]) 	//emulate Roll with Scroll (Z)
			{
				mouseRoll.SetToBeProcessed();
				mouseRoll.SetGain(mouse_z - whc_wheel::c_mouse_z_prev); //Instant mouvement of the wheel : -1, 0, +1 // Yield is updated in the same time
				mouseRoll.SetLevel(mouseRoll.level() + mouseRoll.gain()); // level is not automatically set to 0 when event processed (mouse_w will be : see case default)
				whc_wheel::c_mouse_z_prev = mouse_z;

				if ((key[KEY_LCONTROL]))
				{
					mouseRoll.SetSpeed(mouseRoll.speed() + mouseRoll.gain());	//Instant acceleration
				}
				else
				{
					mouseRoll.SetSpeed(0);
				}

			}
			else					//Scroll
			{
				mouseScroll.SetToBeProcessed();
				mouseScroll.SetGain(mouse_z - whc_wheel::c_mouse_z_prev); //Instant mouvement of the wheel : -1, 0, +1 // Yield is updated in the same time
				mouseScroll.SetLevel( mouseScroll.level() + mouseScroll.gain()); // level is not automatically set to 0 when event processed (mouse_z will be : see case default)
				whc_wheel::c_mouse_z_prev = mouse_z;

				if ((key[KEY_LCONTROL]))
				{
					mouseScroll.SetSpeed(mouseScroll.speed() + mouseScroll.gain());	//Instant acceleration
				}
				else
				{
					mouseScroll.SetSpeed(0);
				}
			}

			break;

    	case MOUSE_FLAG_MOVE_W  :   //Roll
    		mouseRoll.SetToBeProcessed();
    		mouseRoll.SetGain(mouse_w - whc_wheel::c_mouse_w_prev); //Instant mouvement of the wheel : -1, 0, +1 // Yield is updated in the same time
    		mouseRoll.SetLevel(mouseRoll.level() + mouseRoll.gain()); // level is not automatically set to 0 when event processed (mouse_w will be : see case default)
    		whc_wheel::c_mouse_w_prev = mouse_w;

			if ((key[KEY_LCONTROL]))
			{
				mouseRoll.SetSpeed(mouseRoll.speed() + mouseRoll.gain());	//Instant acceleration
			}
			else
			{
				mouseRoll.SetSpeed(0);
			}
			break;

		default :
			if (mouseScroll.isProcessed() && (not (mouse_z==0))) {position_mouse_z(0);}
			if (mouseRoll.isProcessed() && (not (mouse_w==0))) {position_mouse_w(0);}
			break;
    }
}

// SANS ACCELERATION
void whc_wheel::c_leveIncrease(whc_wheel &wheel, int &i_level, int maxlevel, int minlevel)
{
	i_level = i_level + wheel.yield();
	wheel.SetProcessed();

	if (i_level > maxlevel)
	{
		i_level=maxlevel;
	}
	if (i_level<minlevel)
	{
		i_level=minlevel ;
	}
}

int whc_wheel::c_leveIncrease(whc_wheel &wheel, float &f_level, float maxlevel, float minlevel)
{
    f_level = f_level + wheel.yield();
    wheel.SetProcessed();

    if (f_level > maxlevel)
    {
        f_level=maxlevel;
    }
    if (f_level<minlevel)
    {
        f_level=minlevel ;
    }
    return (int) f_level;
}

// Modifie la valeur de la variable float fournie par r�f�rence : f_level
// et retourne la part enti�re sign�e (int)

// AUGMENTATION selon SPEED et le nombre de passage dans la boucle --
// Apr�s un nombre de passage dans la boucle (loopsbystep) le niveau (level) est augmentation/diminu� selon (speed) courant
void whc_wheel::c_leveIncrease(whc_wheel &wheel, int &i_level, int maxlevel, int minlevel, int &i_loops, int loopsbeforeincrease)
{
    i_loops ++;

	if (i_loops>=loopsbeforeincrease)
	{
		i_loops = 1;
		i_level = i_level + wheel.speed();

		if (i_level > maxlevel)
		{
			i_level=maxlevel;
		}
		if (i_level<minlevel)
		{
			i_level=minlevel ;
		}
		wheel.isProcessed();
	}
}

int whc_wheel::c_leveIncrease(whc_wheel &wheel, float &f_level, float maxlevel, float minlevel, float speedratio)
{
    float f_speedsubdivision = wheel.speed() / speedratio ;
    f_level = f_level + f_speedsubdivision;

    if (f_level > maxlevel)
    {
        f_level=maxlevel;
    }
    if (f_level<minlevel)
    {
        f_level=minlevel ;
    }
    return (int) f_level;
}


//---------------------------------------------------------------------------------------------------------------------------------------


whc_pointer::whc_pointer() : m_x(0), m_y(0)
{
	//ctor
}

whc_pointer::whc_pointer(volatile int& mouse_x, volatile int& mouse_y)
{
	//ctor
	m_x = mouse_x ;
	m_y = mouse_y ;
}
whc_pointer::~whc_pointer()
{
	//dtor
}

