#include "shrd_ptr.h"
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const int INTPOISON = -66666;
const int CellSize = 50;
const int MAPSIZE = 20;

const char PLAYER ='I';
const char ENEMY  ='&';
const char WALL   ='X';
const char FREE   ='.';


void startcom ();
int get_sign (int val);

void gm_prc ();

//-----------------------------------------------------------------------------
template <typename T = double>
class Vector
    {
    public:
    T x, y;

    //-----------

    Vector ();
    Vector (T a, T b);
   ~Vector ();
    Vector (POINT point);


    Vector <T> Normalize () const;
    double Angle () const;
    double Len   () const;

    Vector <T> operator + (Vector <T> that ) const;
    Vector <T> operator - (Vector <T> that) const;
    Vector <T> operator / (T divider) const;
    Vector <T> operator * (T multiplier) const;
    };
typedef Vector <int> vec;

struct eng;
struct ob
    {
    enum TYPE {HIDER = 'I', CHASER = '&'};
    TYPE type_;
    vec coords_;
    eng* eng_;
    int status_;
    //----------------
    ob () = delete;
    ob (TYPE type, eng* eng, vec coords);
    void dump ();
    virtual ~ob () = 0;
    virtual void logic () = 0;
    virtual void moove () = 0;
    };

struct hider  : ob
    {
    enum GMPROGRESS {LOSE, WIN, CONTINUE};

    GMPROGRESS gmProgress;
    int LastKeyState;
    //------------------
    hider (eng* motor, vec coords);
    virtual ~hider () override;

    virtual void logic () override;
    virtual void moove () override;
    };

struct chaser : ob
    {

    shrd_ptr<hider> aim_;
    //-------------------
    chaser (eng* eng, shrd_ptr<hider> aim, vec coords) ;
    virtual ~chaser () override;
    virtual void logic () override;
    virtual void moove () override;
    };

struct Map
    {
    char defMap_ [MAPSIZE][MAPSIZE];
    char map_ [MAPSIZE][MAPSIZE];
    //------------------
    Map ();
    Map (char ar [MAPSIZE][MAPSIZE+1]);

    void PlaceDefault();
    void Clear ();
    void draw ();
    };

struct eng
    {
    std::vector<shrd_ptr<ob>> ar_;
    Map* map_;
    //---------------------------------
    eng (Map* _map);
   ~eng ();
    eng (const eng&) = delete;
    eng& operator = (const eng&) = delete;
    void updateMap();
    void   skanMap();
    void add (shrd_ptr<ob>);
    shrd_ptr<ob> findPlayer();
    void run ();
    void draw();
    void dump();
    };

//-----------------------------------------------------------------------------


int main ()
    {
    startcom();
    gm_prc ();

    }

void gm_prc ()
    {
    char yakarta [MAPSIZE][MAPSIZE+1] =
                  {"....................",
                   ".....&..............",
                   "....................",
                   ".....&..............",
                   "....................",
                   "....................",
                   "....................",
                   "....................",
                   "....................",
                   "....................",
                   "....................",
                   "....................",
                   ".&..................",
                   "...........I........",
                   "....................",
                   "....................",
                   "......&.............",
                   "....................",
                   "....................",
                   "...................."};
    Map karta = yakarta;
    eng eng_ (&karta);
    eng_.dump();
    {
    shrd_ptr<hider> player (eng_.findPlayer());

    while (  !(GetAsyncKeyState(VK_SPACE) || player->gmProgress == hider::GMPROGRESS::LOSE)  )
        {
        eng_.run();
        txSleep(120);
        }
    eng_.dump();
    printf ("***\n");
    printf ("size %d", eng_.ar_.size());
    }
    }

//{Map::-----------------------------------------------------------------------
Map::Map()
    {
    Clear();
    memset (defMap_, '.', MAPSIZE*MAPSIZE);
    }

//-----------------------------------------------------------------------------
Map::Map (char defAr [MAPSIZE][MAPSIZE+1])
    {
    for (int y = 0; y < MAPSIZE; y++)
        for (int x = 0; x < MAPSIZE; x++)
            {
            defMap_[y][x] = defAr[y][x];
            }
    Clear();
    }

//-----------------------------------------------------------------------------
void Map::PlaceDefault()
    {
    for (int y = 0; y < MAPSIZE; y++)
        for (int x = 0; x < MAPSIZE; x++)
            {
            map_[y][x] = defMap_[y][x];
            }
    }

//-----------------------------------------------------------------------------
void Map::Clear ()
    {
    for (int y = 0; y < MAPSIZE; y++)
        for (int x = 0; x < MAPSIZE; x++)
            {
            map_[y][x] = FREE;
            }
    }

//-----------------------------------------------------------------------------
void Map::draw ()
    {
    for (int y = 0; y < MAPSIZE; y++)
        for (int x = 0; x < MAPSIZE; x++)
            {
            switch (map_[y][x])
                {
                case PLAYER :
                    {
                    txSetColor(TX_LIGHTGREEN);
                    txSetFillColor(TX_GREEN);
                    break;
                    }
                case ENEMY  :
                    {
                    txSetColor(TX_LIGHTRED);
                    txSetFillColor(TX_RED);
                    break;
                    }
                case WALL   :
                    {
                    txSetColor(TX_LIGHTGRAY);
                    txSetFillColor(TX_GRAY);
                    break;
                    }
                case FREE   :
                    {
                    txSetColor(TX_LIGHTGRAY);
                    txSetFillColor(TX_GRAY);
                    break;
                    }
                default :
                    {
                    printf ("your map contains dryann' (%3d, %3d - %3d or %c) \n", y, x, map_[y][x], map_[y][x]);
                    txSetColor(TX_BROWN);
                    txSetFillColor(TX_BROWN);
                    break;
                    }

                }
            txRectangle (x*CellSize, y*CellSize, (x+1)*CellSize, (y+1)*CellSize);
            }
    }

//-----------------------------------------------------------------------------



//}
//-----------------------------------------------------------------------------



//{eng::------------------------------------------------------------------------
eng::eng (Map* _map) :
    map_ (_map),
    ar_ ()
    {
    skanMap();
    }

//-----------------------------------------------------------------------------
eng::~eng ()
    {}


//-----------------------------------------------------------------------------
void eng::skanMap()
    {
    //txMessageBox("û");
    hider* poiman_no_ne_clomlen = nullptr;

    for (int y = 0; y < MAPSIZE; y++)
        for (int x = 0; x < MAPSIZE; x++)
            {
            auto& obj = map_->defMap_[y][x];

            if (obj == PLAYER)
                {
                hider* plyr = new hider (this, vec (x, y));
                poiman_no_ne_clomlen = plyr;
                add (plyr);
                obj = FREE;
                }
            if (obj == ENEMY)
                {
                chaser* chsr = new chaser (this, nullptr,  vec (x, y));
                add (chsr);
                obj = FREE;
                }
            }

    for (auto& obj : ar_)
        {
        if (obj->type_ == ENEMY)
            {
            dynamic_cast<chaser*>(&obj)->aim_ = shrd_ptr<hider>(poiman_no_ne_clomlen);
            }
        }
    }
//-----------------------------------------------------------------------------
void eng::updateMap()
    {
    map_->Clear();
    map_->PlaceDefault();
    for (auto& obj : ar_)
        {
        map_->map_[obj->coords_.y][obj->coords_.x] = obj->type_;
        }
    }
//-----------------------------------------------------------------------------
void eng::run ()
    {
    updateMap();
    draw();
    for (auto& obj : ar_)
        {
        obj->logic();
        obj->moove();
        assert (obj->status_ == 0);
        }

    }
//-----------------------------------------------------------------------------
shrd_ptr<ob> eng::findPlayer()
    {
    //int nPlrs = 0;
    //ob* Plyr = nullptr;
    for (auto& obj : ar_)
        {
        if (obj->type_ == PLAYER)
            {
            //nPlrs++;
            //Plyr = &obj;
            return obj;
            }
        }
    //assert (nPlrs == 1);
    //return Plyr;
    }
//-----------------------------------------------------------------------------
void eng::add (shrd_ptr<ob> obj)
    {
    ar_.push_back(obj);
    }

//-----------------------------------------------------------------------------
void eng::draw()
    {
    map_->draw();
    }

//-----------------------------------------------------------------------------
void eng::dump ()
    {
    for (int i = 0; i < ar_.size(); i++)
        printf (" num[%d] type[%c] \n",i, ar_.at(i)->type_);
    printf("\n");
    }

//}
//-----------------------------------------------------------------------------



//{ob::------------------------------------------------------------------------
ob::ob (TYPE type, eng* eng, vec coords) :
    type_ (type),
    coords_ (coords),
    eng_ (eng),
    status_(0)
    {}

//-----------------------------------------------------------------------------
ob::~ob ()
    {
    assert (status_ == 0);
    status_ = -666;
    }

//-----------------------------------------------------------------------------
void ob::dump ()
    {

    printf ("status  = %d\n", status_);
    }
//}
//-----------------------------------------------------------------------------



//{hider::---------------------------------------------------------------------

//-----------------------------------------------------------------------------
hider::hider (eng* motor, vec coords) :
    ob (HIDER, motor, coords),
    gmProgress (CONTINUE),
    LastKeyState (0)
    {}

//-----------------------------------------------------------------------------

void hider::logic ()
    {
    if (coords_.x == 0 && coords_.y == 0) gmProgress = WIN;
    else gmProgress = CONTINUE;

    }

//-----------------------------------------------------------------------------
/*void hider::moove2 ()
    {
    if (GetAsyncKeyState ( VK_UP   )  && coords_.y-1 >= 0
                                      && assert(0 <= coords_.y-1 && coords_.y-1 < MAPSIZE)
                                      && eng_->map_->map_[coords_.y-1][coords_.x] == FREE)
        {
        coords_.y --;
        }
    if (GetAsyncKeyState ( VK_DOWN )  && coords_.y < MAPSIZE-1 && eng_->map_->map_[coords_.y + 1][coords_.x] == FREE)
        {
        coords_.y ++;
        }
    if (GetAsyncKeyState ( VK_LEFT )  &&    coords_.x > 0      && eng_->map_->map_[coords_.y][coords_.x-1] == FREE)
        {
        coords_.x --;
        }
    if (GetAsyncKeyState ( VK_RIGHT ) && coords_.x < MAPSIZE-1 && eng_->map_->map_[coords_.y][coords_.x+1] == FREE)
        {
        coords_.x ++;
        }

    LastKeyState = 0;
    }
  */
//-----------------------------------------------------------------------------

void hider::moove ()
    {
    vec tempcords = coords_;

    if (GetAsyncKeyState ( VK_UP   ) )
        {
        tempcords.y --;
        }
    if (GetAsyncKeyState ( VK_DOWN ) )
        {
        tempcords.y ++;
        }
    if (GetAsyncKeyState ( VK_LEFT ) )
        {
        tempcords.x --;
        }
    if (GetAsyncKeyState ( VK_RIGHT ))
        {
        tempcords.x ++;
        }

    if (0 <= tempcords.y && tempcords.y < MAPSIZE &&
        0 <= tempcords.x && tempcords.x < MAPSIZE)
        coords_ = tempcords;

    }

//-----------------------------------------------------------------------------

hider::~hider ()
    {}

//}
//-----------------------------------------------------------------------------



//{chaser::--------------------------------------------------------------------
chaser::chaser (eng* eng, shrd_ptr<hider> aim, vec coords) :
    ob (CHASER, eng, coords),
    aim_ (aim)
    {}

//-----------------------------------------------------------------------------
chaser::~chaser ()
    {}
//-----------------------------------------------------------------------------
void chaser::logic ()
    {
    if (abs(coords_.x - aim_->coords_.x) <= 1 && abs(coords_.y - aim_->coords_.y) <= 1 ) aim_->gmProgress = hider::LOSE;

    }

//-----------------------------------------------------------------------------
void chaser::moove ()
    {
    vec delta (aim_->coords_.x - coords_.x, aim_->coords_.y - coords_.y);
    if (abs(delta.x) > abs(delta.y)) coords_.x += get_sign(delta.x);
                    else             coords_.y += get_sign(delta.y);
    }


//}
//----------------------------------------------------------------------------



//{ Vector::

template <typename T>
Vector <T>::~Vector ()
    {
    x = INTPOISON;
    y = INTPOISON;
    }

//-----------------------------------------------------------------------------

template <typename T>
Vector <T>::Vector () :
    x (0),
    y (0)
    {
    }

//-----------------------------------------------------------------------------
template <typename T>
Vector <T>::Vector (POINT point) :
    x (point.x),
    y (point.y)
    {
    }

//-----------------------------------------------------------------------------

template <typename T>
Vector <T>::Vector (T a, T b) :
    x (a),
    y (b)
    {
    }

//-----------------------------------------------------------------------------
template <typename T>
Vector <T> Vector <T>::operator - (Vector <T> that) const
                                                                            //Vector C = Minus (a, b);
                                                                            //Vector C = a.Minus (b);
    {
    Vector <T> res (x - that.x, y - that.y);

    return res;
    }


//-----------------------------------------------------------------------------
template <typename T>
Vector <T> Vector <T>::operator + (Vector <T> that ) const
                                                                            //Vector C = Minus (a, b);
                                                                            //Vector C = a.Minus (b);
    {
    Vector <T> res (this->x + that.x, this->y + that.y);

    return res;
    }
//-----------------------------------------------------------------------------

template <typename T>
Vector <T> Vector <T>::operator / (T divider) const
                                                                            //Vector C = Minus (a, b);                                                                         //Vector C = a.Minus (b);
    {
    Vector <T> res (x/divider, y/divider);

    return res;
    }


//-----------------------------------------------------------------------------

template <typename T>
double Vector <T>::Len () const
    {
    double VectorLen = hypot (x, y);
    return VectorLen;
    }

//-----------------------------------------------------------------------------
template <typename T>
Vector <T> Vector <T>::Normalize () const
    {
    double len = Len();
    if (len < DBL_EPSILON)
        return Vector <T> (0, 0);

    Vector <T> NormalizedVector = *this/len;
    return NormalizedVector;

    }

//-----------------------------------------------------------------------------
template <typename T>
Vector <T> Vector <T>::operator * (T multiplier) const
    {
    Vector <T> res (x * multiplier, y * multiplier);

    return res;
    }


//-----------------------------------------------------------------------------
template <typename T>
double Vector <T>::Angle () const
    {
    vec promezshootochnoe_nazvanie = this->Normalize ();
    double angle = acos (promezshootochnoe_nazvanie.x);

    angle = angle/M_PI*180;
    if (y > 0) angle = -angle + 360;

    return angle;
    }


//}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void startcom ()
    {
    txCreateWindow (MAPSIZE*CellSize, MAPSIZE*CellSize);
    txSetColor (TX_WHITE);
    txSetFillColor (TX_WHITE);
    }

//-----------------------------------------------------------------------------
int get_sign (int val)
    {
    return (val < 0)? -1 : (val > 0)? 1 : 0;        //return !!(val & 0x80000000)     1101011101101111010110000111
    }                                                                               //1000000000000000000000000000
                                                                                    //1000000000000000000000000000
//-----------------------------------------------------------------------------







