
#include "pixelmachine.h"



PIXELMACHINE::PIXELMACHINE()
{
    cancel = false;
    running = false;
    img = NULL;
    dimg = NULL;
    reg_count = 0;
    reg_alloc = 8;
    regions = (REG*)malloc(sizeof(REG)*reg_alloc);
    lock = SDL_CreateMutex();

    init();
}


PIXELMACHINE::~PIXELMACHINE()
{
    SDL_DestroyMutex(lock);
    if(img)
        free(img);
    free(regions);
}


void PIXELMACHINE::init()
{
    int i,j;

    for(i=0; i<BSIZE; i++)
        for(j=0; j<BSIZE; j++)
            memset( blocks[i][j], 0, BSIZE*sizeof(COLOR) );
    blocksize = 1000.0/((double)BSIZE);

    w = W;
    h = H;
    multis = MULTIS;
    threads = THREADS;
    frames = FRAMES;

    seed = (unsigned)time(NULL);

    build_terrain();
    generate_objects();
}




void PIXELMACHINE::build_terrain()
{
    int i,j,k;

#ifndef PHOTONMODE
    int div = TERDIV;
    for(i=0;i<BSIZE;i+=div)
        for(j=0;j<BSIZE;j+=div)
        {
            terrain[i][j] = (double)(rand()%1000)*0.33-200.0;
        }
    while( div>1 )
    {
        for(i=div/2;i<BSIZE;i+=div)
            for(j=div/2;j<BSIZE;j+=div)
            {
                terrain[i][j] = (terrain[i-div/2][j-div/2] + terrain[i-div/2][j+div/2] + terrain[i+div/2][j-div/2] + terrain[i+div/2][j+div/2])*0.25;
            }

        for(i=0;i<BSIZE;i+=div/2)
            for(j=0;j<BSIZE;j+=div/2)
            {
                if( (i+j)%div )
                {
                    if( i==0 )
                        terrain[i][j] = (                      terrain[i+div/2][j] + terrain[i][j-div/2] + terrain[i][j+div/2])*0.33333;
                    else if( i==BSIZE-1 )
                        terrain[i][j] = (terrain[i-div/2][j]                       + terrain[i][j-div/2] + terrain[i][j+div/2])*0.33333;
                    else if( j==0 )
                        terrain[i][j] = (terrain[i-div/2][j] + terrain[i+div/2][j]                       + terrain[i][j+div/2])*0.33333;
                    else if( j==BSIZE-1 )
                        terrain[i][j] = (terrain[i-div/2][j] + terrain[i+div/2][j] + terrain[i][j-div/2]                      )*0.33333;
                    else
                        terrain[i][j] = (terrain[i-div/2][j] + terrain[i+div/2][j] + terrain[i][j-div/2] + terrain[i][j+div/2])*0.25;
                }
            }

        div /= 2;
    }
#endif

    for(k=0; k<BSIZE; k++)
        for(i=0; i<BSIZE; i++)
            for(j=0; j<BSIZE; j++)
            {
#ifndef PHOTONMODE
                if( k==0 )
                {
#endif
                    blocks[i][j][k].r = (double)(rand()%80)/255.0 + 0.70;
                    blocks[i][j][k].g = (double)(rand()%60)/255.0 + 0.60;
                    blocks[i][j][k].b = (double)(rand()%40)/255.0 + 0.20;
                    blocks[i][j][k].a = 1.0; //(double)(rand()%100+1)*0.01;
#ifndef PHOTONMODE
                }
                else if( i>(BSIZE*2)/5 && i<(BSIZE*3)/5 && j>(BSIZE*2)/5 && j<(BSIZE*3)/5 ) // shiny area!
                {
                    if( k<BSIZE/5 )
                        blocks[i][j][k] = blocks[i][j][k-1];
                    else
                    {
                        blocks[i][j][k].r = 0.0;
                        blocks[i][j][k].g = 0.0;
                        blocks[i][j][k].b = 0.0;
                        blocks[i][j][k].a = 0.0;
                    }
                }
                else if( terrain[i][j] >= k*(1000.0/(double)BSIZE ) )
                {
                    blocks[i][j][k] = blocks[i][j][k-1];
                }
                else
                {
                    blocks[i][j][k].r = 0.0;
                    blocks[i][j][k].g = 0.0;
                    blocks[i][j][k].b = 0.0;
                    blocks[i][j][k].a = 0.0;
                }
#endif
            }
}


void PIXELMACHINE::generate_objects()
{
    int i;

#ifndef PHOTONMODE
    sun[0].x = (double)(rand()%1500) - 250.0;
    sun[0].y = (double)(rand()%1500) - 250.0;
    sun[0].z = (double)(rand()%700) + 100.0;

    for(i=0; i<SUNS; i++)
    {
        sun[i].x = sun[0].x + ((double)i)*0.4;
        sun[i].y = sun[0].y + ((double)i)*0.3;
        sun[i].z = sun[0].z + ((double)i)*0.2;
    }

    cam.x = (double)(rand()%900) + 50.0;
    cam.y = (double)(rand()%900) + 50.0;
    cam.z = terrain[int(cam.x/SQSIZE)][int(cam.y/SQSIZE)] + (double)(rand()%150) + 50.0;

    tar.x = (double)(rand()%900) + 50.0;
    tar.y = (double)(rand()%900) + 50.0;
    tar.z = cam.z - (double)(rand()%400);

    for(i=0; i<SPHERES; i++)
    {
        sphere[i].center.x = (double)(rand()%10000)*0.1;
        sphere[i].center.y = (double)(rand()%10000)*0.1;
        sphere[i].center.z = (double)(rand()%3500)*0.1;
        sphere[i].radius = (double)(rand()%10000)*0.005;
        sphere[i].color.a = 1.0;
        sphere[i].color.r = (double)(rand()%255)/255.0;
        sphere[i].color.g = (double)(rand()%255)/255.0;
        sphere[i].color.b = (double)(rand()%255)/255.0;
    }
#endif

#ifdef PHOTONMODE
    cam.x = 778.0;
    cam.y = 621.0;
    cam.z = 631.0;
    tar.x = 625.0;
    tar.y = 625.0;
    tar.z = 624.0;
    sun[0].x = 625.0;
    sun[0].y = 625.0;
    sun[0].z = 749.0;
    sun[1].x = 998.0;
    sun[1].y = 721.0;
    sun[1].z = 590.0;

    blocks[2][2][2].a = 0.0;

    //black
    blocks[3][2][2].r = 0.2; blocks[3][2][2].g = 0.15; blocks[3][2][2].b = 0.1;
    //main wall
    blocks[1][2][2].r = 1.2; blocks[1][2][2].g = 1.1; blocks[1][2][2].b = 0.70;
    //floor
    blocks[2][2][1].r = 1.2; blocks[2][2][1].g = 1.1; blocks[2][2][1].b = 0.70;
    //ceiling
    blocks[2][2][3].r = 0.8; blocks[2][2][3].g = 0.7; blocks[2][2][3].b = 0.5;
    // right wall
    blocks[2][3][2].r = 0.4; blocks[2][3][2].g = 0.55; blocks[2][3][2].b = 0.35;
    // left wall
    blocks[2][1][2].r = 0.8; blocks[2][1][2].g = 0.5; blocks[2][1][2].b = 0.3;


    sphere[0].center.x = 594.0;
    sphere[0].center.y = 595.0;
    sphere[0].center.z = 702.0;
    sphere[0].radius = 25.0;

    sphere[1].center.x = 540.0;
    sphere[1].center.y = 728.0;
    sphere[1].center.z = 540.0;
    sphere[1].radius = 40.0;

    sphere[2].center.x = 585.0;
    sphere[2].center.y = 584.0;
    sphere[2].center.z = 510.0;
    sphere[2].radius = 10.0;

    sphere[3].center.x = 540.0;
    sphere[3].center.y = 730.0;
    sphere[3].center.z = 620.0;
    sphere[3].radius = 40.0;

    sphere[4].center.x = 585.0;
    sphere[4].center.y = 584.0;
    sphere[4].center.z = 530.0;
    sphere[4].radius = 10.0;

    sphere[5].center.x = 585.0;
    sphere[5].center.y = 584.0;
    sphere[5].center.z = 550.0;
    sphere[5].radius = 10.0;
#endif
}


void PIXELMACHINE::run()
{
    int i,j,k;
    char str[80];

    running = true;

    if( (PX1-PX0)/(double)PATCHDIM < PHOTONRADIUS )
        printf( "WARNING: PATCH SIZE IS SMALLER THAN PHOTON RADIUS!" );

    for(i=0; i<PATCHDIM; i++ ) for(j=0; j<PATCHDIM; j++ ) for(k=0; k<PATCHDIM; k++ )
    {
        patch[i][j][k]=NULL;
        patchterm[i][j][k] = &(patch[i][j][k]);
    }

    srand(seed);
    
    busy = (bool*)malloc(sizeof(bool)*threads);


    V lin;
    subtract( lin, tar, cam );
    normalize( lin, 2.0 );

    
    for(i=0;i<frames;i++)
    {
        //system(SYSTEMCLEARSTR);
        //printf("Previewing...\n");
        //render(PREVIEWW,PREVIEWH,true);
        //preview(PREVIEWW,PREVIEWH);
        //savebmp("./preview.bmp",PREVIEWW,PREVIEWH);
        //printf("<Space> render   <W/A/S/D/R/F> camera   <I/K/J/L/P/;> target\n");
        //ch = _getch();
        
        //printf("Rendering...\n");
#ifndef PHOTONMODE
        render(w,h,false);
#else
        render_photons(cam,tar,w,h,false);
#endif

        printf("Writing frame %.4d ...\n",i);
        if( frames > 1 )
        {
#ifdef unix
            mkdir("./video",0777);
#else
            mkdir("./video");
#endif
            sprintf(str,"./video/frame%.4d.bmp",i);
            savebmp(str,w,h);
        }
        else
        {
#ifdef unix
            mkdir("./images",0777);
#else
            mkdir("./images");
#endif
            unsigned t = (unsigned)time(NULL);
            sprintf(str,"./images/img%d.bmp",t);
            savebmp(str,w,h);
            //sprintf(str,"start ./images/img%d.bmp",t);
            //system(str);
        }

        cam.x -= lin.x/2.0;
        cam.y -= lin.y/2.0;
        cam.z -= lin.z/2.0;
        tar.z -= 0.50;
        cam.z -= 0.45;

        for(j=0;j<SUNS;j++)
            sun[j].z += 0.8;

        for(j=0;j<SPHERES;j++)
        {
            switch( j%3 )
            {
            case 2: sphere[j].center.z += 1.13; break;
            case 1: sphere[j].center.z -= 1.13; break;
            case 0: break;
            }
        }
    }

    free(busy);
    running = false;
}



void PIXELMACHINE::savebmp( const char *filename, int w, int h )
{
    int i;
    FILE *f;
    int filesize = 54 + 3*w*h;

    unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    unsigned char bmppad[3] = {0,0,0};

    bmpfileheader[ 2] = (unsigned char)(filesize    );
    bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
    bmpfileheader[ 4] = (unsigned char)(filesize>>16);
    bmpfileheader[ 5] = (unsigned char)(filesize>>24);

    bmpinfoheader[ 4] = (unsigned char)(       w    );
    bmpinfoheader[ 5] = (unsigned char)(       w>> 8);
    bmpinfoheader[ 6] = (unsigned char)(       w>>16);
    bmpinfoheader[ 7] = (unsigned char)(       w>>24);
    bmpinfoheader[ 8] = (unsigned char)(       h    );
    bmpinfoheader[ 9] = (unsigned char)(       h>> 8);
    bmpinfoheader[10] = (unsigned char)(       h>>16);
    bmpinfoheader[11] = (unsigned char)(       h>>24);

    //f = fopen("img.raw","wb");
    //fwrite(img,3,w*h,f);
    //fclose(f);

    f = fopen(filename,"wb");
    fwrite(bmpfileheader,1,14,f);
    fwrite(bmpinfoheader,1,40,f);
    for(i=0; i<h; i++)
    {
        fwrite(img+(w*(h-i-1)*3),3,w,f);
        fwrite(bmppad,1,(4-(w*3)%4)%4,f);
    }
    fclose(f);
}





void PIXELMACHINE::render( int w, int h, bool quiet )
{
    int i,j;
    int ret;

    V mycam = cam;
    V mytar = tar;

    SDL_Thread **kids;
    kids = (SDL_Thread**)malloc(sizeof(SDL_Thread*)*threads);
    memset(kids,0,sizeof(SDL_Thread*)*threads);

    RENDER_ARGS *ra;
    ra = (RENDER_ARGS*)malloc(sizeof(RENDER_ARGS)*threads);

    if( img )
        free( img );
    img = (unsigned char *)malloc(3*w*h);
    memset(img,0,sizeof(img));

    subtract( mytar, mytar, mycam );
    normalize( mytar, 1.0 );
    add( mytar, mytar, mycam );

    tarw = (double)w/(double)h*1.5;
    pixpitch = tarw/(double)w;
    subpitch = pixpitch/((double)multis+1.0);


    i = 0;
    memset(busy,0,sizeof(bool)*threads);

    for(j=0;j<w;j++)
    {
        while( busy[i] )
        {
            i++;
            if( i%threads == 0 )
            {
                SDL_Delay(10);
                i = 0;
            }
        }

        if( cancel )
            break;

        if( kids[i] )
        {
            SDL_WaitThread(kids[i],&ret);
            push_region(ra+i);
        }

        ra[i].cam = mycam;
        ra[i].tar = mytar;
        ra[i].w = w;
        ra[i].h = h;
        ra[i].l = w/2 + ( j%2 ? (-j-1)/2 : j/2 );
        ra[i].r = ra[i].l+1;
        ra[i].t = 0;
        ra[i].b = h;
        ra[i].quiet = quiet;
        ra[i].lock = lock;
        ra[i].threadid = i;
        ra[i].pm = this;
        busy[i] = true;
        kids[i] = SDL_CreateThread(ext_render_thread,ra+i);
    }

    for(i=0;i<threads;i++)
        if( kids[i] )
        {
            SDL_WaitThread(kids[i],&ret);
            push_region(ra+i);
        }

    free(kids);
    free(ra);
}


void PIXELMACHINE::push_region( RENDER_ARGS *ra )
{
    int i;
    SDL_mutexP(lock);
    for(i=0; i<reg_count; i++) // try to smoosh into existing rectangle
    {
        if( ((regions[i].r == ra->l || regions[i].l == ra->r) && regions[i].t == ra->t && regions[i].b == ra->b)
         || ((regions[i].b == ra->t || regions[i].t == ra->b) && regions[i].l == ra->l && regions[i].r == ra->r) )
        {
            regions[i].l = MIN(regions[i].l,ra->l);
            regions[i].r = MAX(regions[i].r,ra->r);
            regions[i].t = MIN(regions[i].t,ra->t);
            regions[i].b = MAX(regions[i].b,ra->b);
            SDL_mutexV(lock);
            return;
        }
    }
    if( reg_count==reg_alloc ) // need to grow?
        regions = (REG*)realloc(regions,sizeof(REG)*(reg_alloc*=2));
    regions[reg_count].l = ra->l;
    regions[reg_count].r = ra->r;
    regions[reg_count].t = ra->t;
    regions[reg_count].b = ra->b;
    reg_count++;
    SDL_mutexV(lock);
}


bool PIXELMACHINE::pop_region( SDL_Rect *rect )
{
    bool ret = false;

    SDL_mutexP(lock);
    if( reg_count>0 )
    {
        reg_count--;
        rect->x = regions[reg_count].l;
        rect->y = regions[reg_count].t;
        rect->w = regions[reg_count].r - regions[reg_count].l;
        rect->h = regions[reg_count].b - regions[reg_count].t;
        ret = true;
    }
    SDL_mutexV(lock);
    return ret;
}


int ext_render_thread( void *data )
{
    PIXELMACHINE::RENDER_ARGS *ra = (PIXELMACHINE::RENDER_ARGS*)data;
    return ra->pm->render_thread( data );
}


int PIXELMACHINE::render_thread( void *data )
{
    int i,j,k,m,n;
    double di,dj;
    double r,g,b;
    int subgoal;

    RENDER_ARGS *ra = (RENDER_ARGS*)data;
    if( ra->b > ra->h ) ra->b = ra->h;
    if( ra->b < 0     ) ra->b = 0;
    if( ra->t > ra->h ) ra->t = ra->h;
    if( ra->t < 0     ) ra->t = 0;
    if( ra->l > ra->w ) ra->l = ra->r;
    if( ra->l < 0     ) ra->l = 0;
    if( ra->r > ra->w ) ra->r = ra->r;
    if( ra->r < 0     ) ra->r = 0;

    COLOR *color;
    color = (COLOR*)malloc(sizeof(COLOR)*(multis*multis));

    V lin;
    lin.x = ra->tar.x - ra->cam.x;
    lin.y = ra->tar.y - ra->cam.y;
    lin.z = ra->tar.z - ra->cam.z;

    subgoal = 0;

    for(i=ra->l; i<ra->r; i++)
    {
        for(j=ra->t; j<ra->b; j++)
        {
            di = (double)i;
            dj = (double)j;

            for(m=0; m<multis; m++)
                for(n=0; n<multis; n++)
                {
                    k = m + n*multis;

                    V wing;
                    wing.x =  lin.y;
                    wing.y = -lin.x;
                    wing.z = 0.0;

                    V head;
                    cross( head, lin, wing );

                    normalize( head, (dj-((double)ra->h)/2.0)*pixpitch + subpitch*(double)n );
                    normalize( wing, (di-((double)ra->w)/2.0)*pixpitch + subpitch*(double)m );

                    V ray;
                    ray.x = ra->tar.x+wing.x+head.x - ra->cam.x;
                    ray.y = ra->tar.y+wing.y+head.y - ra->cam.y;
                    ray.z = ra->tar.z+wing.z+head.z - ra->cam.z;

                    raytrace( color[k], ra->cam, ray, MODE_CAMRAY, 0 );
                }

            r = 0.0;
            g = 0.0;
            b = 0.0;

            for(k=0;k<(multis*multis);k++)
            {
                r += color[k].r;
                g += color[k].g;
                b += color[k].b;
            }

            r /= (double)(multis*multis);
            g /= (double)(multis*multis);
            b /= (double)(multis*multis);

            if( cancel )
            {
                   busy[ra->threadid] = false;
                return 1;
            }

            img[(i+j*ra->w)*3+2] = (unsigned char)(255.0*r+0.5);
            img[(i+j*ra->w)*3+1] = (unsigned char)(255.0*g+0.5);
            img[(i+j*ra->w)*3+0] = (unsigned char)(255.0*b+0.5);
        }
    }

    busy[ra->threadid] = false;
    return 1;
}




void PIXELMACHINE::render_photons( V cam, V tar, int w, int h, bool quiet )
{
    int i,j,k,m,n;
    COLOR c;

    if( img )
        free( img );
    img = (unsigned char *)malloc(sizeof(unsigned char)*3*w*h);
    memset(img,0,sizeof(unsigned char)*3*w*h);

    dimg = (COLOR *)malloc(sizeof(COLOR)*w*h);
    memset(dimg,0,sizeof(COLOR)*w*h);

    subtract( tar, tar, cam );
    normalize( tar, 1.0 );
    add( tar, tar, cam );

    tarw = 2.0;
    pixpitch = tarw/(double)w;
    subpitch = pixpitch/((double)multis+1.0);

    V lin;
    lin.x = tar.x - cam.x;
    lin.y = tar.y - cam.y;
    lin.z = tar.z - cam.z;

    double di,dj;
    int subgoal;
    int substep;

    subgoal = 0;

    for(i=0; i<w; i++)
    {
        for(j=0; j<h; j++)
        {
            di = (double)i;
            dj = (double)j;

            for(m=0; m<multis; m++)
                for(n=0; n<multis; n++)
                {
                    k = m + n*multis;

                    V wing;
                    wing.x =  lin.y;
                    wing.y = -lin.x;
                    wing.z = 0.0;

                    V head;
                    cross( head, lin, wing );

                    normalize( wing, (di-(double)w/2.0)*pixpitch + subpitch*m );
                    normalize( head, (dj-(double)h/2.0)*pixpitch + subpitch*n );

                    V ray;
                    ray.x = tar.x+wing.x+head.x - cam.x;
                    ray.y = tar.y+wing.y+head.y - cam.y;
                    ray.z = tar.z+wing.z+head.z - cam.z;

                    raytrace( c, cam, ray, MODE_CAMRAY, 0, j*w+i );
                }

        }
        
        if( !quiet )
            if( (int)(1000*i/w) >= subgoal )
            {
                subgoal += 20;
                if( subgoal%100==0 )
                    printf("%d",subgoal/10);
                else
                    printf(".");
                fflush(stdout);
            }
    }
    if( !quiet )
        printf("\n");

    
    

    // blast photons around and feed the camera
    c.r = 1.0;
    c.g = 1.0;
    c.b = 1.0;
    c.a = 1.0;
    substep = PHOTONS/50;
    subgoal = substep;
    for(i=0; i<PHOTONS; i++)
    {
        V o;
        o.x = sun[0].x + (double)rand()*INVRAND*40.0-20.0;
        o.y = sun[0].y + (double)rand()*INVRAND*20.0-10.0;
        o.z = sun[0].z;
        V v;
        do
        {
            v.x = (double)rand()*INVRAND-1.0;
            v.y = (double)rand()*INVRAND-1.0;
            v.z = (double)rand()*INVRAND*(-0.5);
        }
        while( v.x*v.x + v.y*v.y + v.z*v.z > 1.0 );

        raytrace(c,o,v,MODE_PHOTON,0);

        if( !quiet )
            if( i >= subgoal )
            {
                subgoal += substep;
                if( (i/substep)%5==0 )
                    printf("%d",(i/substep)*2);
                else
                    printf(".");
                fflush(stdout);
            }
    }
    if( !quiet )
        printf("\n");


    // adjust dimg vals (HDR-like) and copy to bitmap
    double max = 0.0;
    for(i=0; i<w*h; i++)
    {
        if( dimg[i].r > max )
            max = dimg[i].r;
        if( dimg[i].g > max )
            max = dimg[i].g;
        if( dimg[i].b > max )
            max = dimg[i].b;
    }
    double scalar = 255.0/max;
    for(i=0; i<w*h; i++)
    {
        img[i*3+2] = (unsigned char)(dimg[i].r*scalar);
        img[i*3+1] = (unsigned char)(dimg[i].g*scalar);
        img[i*3+0] = (unsigned char)(dimg[i].b*scalar);
    }
    //_getch();

    free(dimg);
}





COLOR &PIXELMACHINE::raytrace( COLOR &color, const V &cam, const V &ray, int mode, int bounce, int pixelindex )
{
    int i,k;
    double r,g,b,a;
    int blockx,blocky,blockz;
    V hit0,hit1,hit2;
    V hit;
    double p0,p1,p2;
    double p,potp;
    int side;
    int hitobject,pothitobject;

    if( bounce >= MAXBOUNCE )
    {
        color.r = 1.0;
        color.g = 0.0;
        color.b = 0.0;
        color.a = 1.0;
        return color;
    }

    side = XY;

    if( mode!=MODE_PHOTON )
    {
        r = 0.0;
        g = 0.0;
        b = 0.0;
        a = 0.0;
    }


    // spheres
    potp = 99999999.0;
    hitobject = -1;
    pothitobject = -1;
    for(i=0; i<SPHERES; i++)
    {
        V v;
        v = ray;
        CollideRaySphere(p0,cam,v,sphere[i].center,sphere[i].radius);

        if(p0!=NO_COLLISION && p0+0.00001 < potp)
        {
            pothitobject = i;
            potp = p0+0.00001;
        }
    }


    p = 0.0;
    k = 0;
    while( 1 )
    {
        double sidefactor;

        // find current space block
        blockx = (int)((cam.x+p*ray.x)/blocksize);
        blocky = (int)((cam.y+p*ray.y)/blocksize);
        blockz = (int)((cam.z+p*ray.z)/blocksize);

        // hit sky
        if( blockx<1 || blockx>BSIZE-1 || blocky<1 || blocky>BSIZE-1 || blockz<0 || blockz>BSIZE-1 || k>1000 || (mode==MODE_LIGHTTEST && p>=1.0) )
        {
            if( mode==MODE_CAMRAY && pixelindex==NO_PIXEL )
            {
                double dot = (ray.x*ray.x + ray.y*ray.y);
                double ang = fabs(dot/( sqrt(dot) * sqrt(ray.x*ray.x + ray.y*ray.y + ray.z*ray.z) ));
                ang *= ang;
                r = r + 0.5*ang*(1.0-a);
                g = g + 0.6*ang*(1.0-a);
                b = b + 1.0*ang*(1.0-a);
                a = 1.0;
            }
            break;
        }

        k++;

        // pickup color from current space block
        COLOR *c;
        if( hitobject>=0 )
            c = &sphere[hitobject].color;
        else
            c = &blocks[blockx][blocky][blockz];

        if( k>1 && (c->a>0.0 || hitobject>=0 ) )
        {
            V beam;
            COLOR light;
            COLOR finallight;

            if( mode!=MODE_PHOTON && pixelindex==NO_PIXEL ) // NOT photoning, NOT building a reverse photon map
            {
                if( mode==MODE_LIGHTTEST )
                {
                    color.r = 0.0;
                    color.g = 0.0;
                    color.b = 0.0;
                    color.a = 0.7;
                    return color;
                }

                // get incoming light/shadow

                finallight.r = 0.0;
                finallight.g = 0.0;
                finallight.b = 0.0;

                for(int m=0;m<SUNS;m++)
                {
                    subtract( beam, sun[m], hit );
                    raytrace( light, hit, beam, MODE_LIGHTTEST, 0 );

                    double falloff = FALLOFF/(beam.x*beam.x + beam.y*beam.y + beam.z*beam.z);
                    light.r = light.r*0.0+(1.0-light.a)*falloff;
                    light.g = light.g*0.0+(1.0-light.a)*falloff;
                    light.b = light.b*0.0+(1.0-light.a)*falloff;

                    finallight.r += light.r;
                    finallight.g += light.g;
                    finallight.b += light.b;
                }

                finallight.r /= (double)SUNS;
                finallight.g /= (double)SUNS;
                finallight.b /= (double)SUNS;

                sidefactor = (side==XY?1.0:(side==XZ?0.9:0.8));
            }

            if( blockz==0 || hitobject>=0 /**/|| blockx>(BSIZE*2)/5 && blockx<(BSIZE*3)/5 && blocky>(BSIZE*2)/5 && blocky<(BSIZE*3)/5/**/ ) //reflect!
            {
                if( mode!=MODE_PHOTON && pixelindex==NO_PIXEL )
                {
                    finallight.r += 0.7*(1.0-finallight.r);
                    finallight.g += 0.7*(1.0-finallight.g);
                    finallight.b += 0.7*(1.0-finallight.b);
                }

                //if( mode==MODE_PHOTON )
                //{
                //    if( rand()%50==0 )
                //        return color; // i've decided metallic objects absorb 2% of photons
                //}


                V ray2 = ray;

                if( hitobject>=0 )
                {
                    V rad;

                    subtract( rad, hit, sphere[hitobject].center );

                    //mirror( ray2, ray, rad );

                    ray2 = ray;
                    normalize(rad, 2.0*(ray.x*rad.x+ray.y*rad.y+ray.z*rad.z)/sphere[hitobject].radius );
                    subtract(ray2,ray,rad);

                    //ray2.x = -ray2.x;
                    //ray2.y = -ray2.y;
                    //ray2.z = -ray2.z;
                }
                else if( side==XY )
                    ray2.z = -ray.z;
                else if( side==XZ )
                    ray2.y = -ray.y;
                else 
                    ray2.x = -ray.x;

#ifndef PHOTONMODE                
                double scatter;
                if( hitobject>=0 )
                {
                    scatter = 0.0;
                    ray2.x *= 1.0 + (double)(rand()%1000)*scatter;
                    ray2.y *= 1.0 + (double)(rand()%1000)*scatter;
                    ray2.z *= 1.0 + (double)(rand()%1000)*scatter;
                }
                else
                {
                    scatter = SCATTER;
                    ray2.x *= 1.0 + (double)(rand()%1000)*scatter + sin(hit.x)*0.1;
                    ray2.y *= 1.0 + (double)(rand()%1000)*scatter + sin(hit.y)*0.1;
                    ray2.z *= 1.0 + (double)(rand()%1000)*scatter;
                }

                
#endif

                //printf("(%d %d %d)",blockx,blocky,blockz);
                //getch();

                raytrace(color, hit, ray2, mode, bounce+1, pixelindex);
                

                if( mode!=MODE_PHOTON && pixelindex==NO_PIXEL )
                {
                    r = r + color.r*(1.0-a)*sidefactor*finallight.r;
                    g = g + color.g*(1.0-a)*sidefactor*finallight.g;
                    b = b + color.b*(1.0-a)*sidefactor*finallight.b;
                    a = 1.0;
                }

                break;
            }

            if( pixelindex!=NO_PIXEL ) // building pixel map, attach and stop
            {
                attach( pixelindex, hit );
                return color;
            }

            if( mode==MODE_PHOTON )
            {
                COLOR temp;
                temp.r = c->r*color.r;
                temp.g = c->g*color.g;
                temp.b = c->b*color.b;

                feed(temp,hit);


                //INDIRECT/DIFFUSE BOUNCE HERE:
                if( rand()%2==0 )
                {
                    V diffuseray;
                    diffuseray.x = (double)rand()*INVRAND-1.0;
                    diffuseray.y = (double)rand()*INVRAND-1.0;
                    diffuseray.z = (double)rand()*INVRAND-1.0;

                    //temp.r *= 10.0;
                    //temp.g *= 10.0;
                    //temp.b *= 10.0;

                    if( diffuseray.x*ray.x + diffuseray.y*ray.y + diffuseray.z*ray.z > 0.0 )
                    {
                        diffuseray.x = -diffuseray.x;
                        diffuseray.y = -diffuseray.y;
                        diffuseray.z = -diffuseray.z;
                    }
                    raytrace(temp,hit,diffuseray,MODE_PHOTON,bounce+1);
                }
                break;
            }


            r = r + c->r*c->a*(1.0-a)*sidefactor*finallight.r;
            g = g + c->g*c->a*(1.0-a)*sidefactor*finallight.g;
            b = b + c->b*c->a*(1.0-a)*sidefactor*finallight.b;
            a += c->a*(1.0-a);

            if( a>=0.98 )
            {
                a = 1.0;
                break;
            }
        }

        // move to next space block
        hit0.x = (double)blockx*blocksize + (ray.x>0.0?blocksize:0.0);
        hit1.y = (double)blocky*blocksize + (ray.y>0.0?blocksize:0.0);
        hit2.z = (double)blockz*blocksize + (ray.z>0.0?blocksize:0.0);

        p0 = (hit0.x-cam.x)/(ray.x) + 0.00001;
        p1 = (hit1.y-cam.y)/(ray.y) + 0.00001;
        p2 = (hit2.z-cam.z)/(ray.z) + 0.00001;

        if( (int)((cam.x+p0*ray.x)/blocksize)==blockx && (int)((cam.y+p0*ray.y)/blocksize)==blocky && (int)((cam.z+p0*ray.z)/blocksize)==blockz )
            p0 = 99999.0;
        if( (int)((cam.x+p1*ray.x)/blocksize)==blockx && (int)((cam.y+p1*ray.y)/blocksize)==blocky && (int)((cam.z+p1*ray.z)/blocksize)==blockz )
            p1 = 99999.0;
        if( (int)((cam.x+p2*ray.x)/blocksize)==blockx && (int)((cam.y+p2*ray.y)/blocksize)==blocky && (int)((cam.z+p2*ray.z)/blocksize)==blockz )
            p2 = 99999.0;

        if( p0<p || p1<p || p2<p )
            break;

        if( p0<p1 && p0<p2 )
        {
            side = YZ;
            p = p0;
        }
        else if( p1<p2 )
        {
            side = XZ;
            p = p1;
        }
        else
        {
            side = XY;
            p = p2;
        }

/*
        // THE NEW PARALLAX (didn't work, unfortunately)
        if( ray.z<0.0 && rand()%100<98 )
        {
            // find current space block
            blockx = (int)((cam.x+p*ray.x)/blocksize);
            blocky = (int)((cam.y+p*ray.y)/blocksize);
            blockz = (int)((cam.z+p*ray.z)/blocksize);
            if( blockx<1 || blockx>BSIZE-1 || blocky<1 || blocky>BSIZE-1 || blockz<0 || blockz>BSIZE-1 )
            {
                side = XY;
                p = (blocksize-cam.z)/(ray.z) + 0.00001;
            }
        }
*/

/*
        if( blockz>0 && blocks[blockx][blocky][blockz-1].a>0.0 ) // crazy speheres!!
        {
            sphere[SPHERES].center.x = ((double)blockx+0.5)*SQSIZE;
            sphere[SPHERES].center.y = ((double)blocky+0.5)*SQSIZE;
            sphere[SPHERES].center.z = ((double)blockz+0.5)*SQSIZE;
            sphere[SPHERES].radius = SQSIZE*0.4;
            sphere[SPHERES].color.r = 1.0;
            sphere[SPHERES].color.g = 1.0;
            sphere[SPHERES].color.b = 1.0;
            sphere[SPHERES].color.a = 1.0;
            CollideRaySphere(p0,cam,ray,sphere[SPHERES].center,sphere[SPHERES].radius);

            if(p0!=NO_COLLISION && p0+0.00001 < p)
            {
                hitobject = SPHERES;
                side = XY;
                p = p0+0.00001;
            }
        }
/**/

        if( p>potp )
        {
            hitobject = pothitobject;
            side = XY;
            p = potp;
        }


        hit.x = (p-0.00002)*ray.x + cam.x;
        hit.y = (p-0.00002)*ray.y + cam.y;
        hit.z = (p-0.00002)*ray.z + cam.z;

        //printf( "%.4d %.4d %.4d\n", blockx, blocky, blockz );
    }

    if( mode!=MODE_PHOTON )
    {
        if( r>1.0 ) r = 1.0;
        if( g>1.0 ) g = 1.0;
        if( b>1.0 ) b = 1.0;
        if( r<0.0 ) r = 0.0;
        if( g<0.0 ) g = 0.0;
        if( b<0.0 ) b = 0.0;

        color.r = r;
        color.g = g;
        color.b = b;
        color.a = a;
    }

    return color;
}



// attach()
// 
// attaches a screen pixel to a patch cube, so that the
// screen pixel can be found by photons later on
//
void PIXELMACHINE::attach( int pixelindex, const V &hit )
{
    static double dx = (double)PATCHDIM/(PX1-PX0);
    static double dy = (double)PATCHDIM/(PY1-PY0);
    static double dz = (double)PATCHDIM/(PZ1-PZ0);

    if(hit.x<=PX0 || hit.x>=PX1 || hit.y<=PY0 || hit.y>=PY1 || hit.z<=PZ0 || hit.z>=PZ1) // not indexible
        return;

    int x = (int)((hit.x-PX0)*dx);
    int y = (int)((hit.y-PY0)*dy);
    int z = (int)((hit.z-PZ0)*dz);

    P** p = patchterm[x][y][z];
    *p = (P*)malloc(sizeof(P));
    (*p)->v = hit;
    (*p)->i = pixelindex;
    (*p)->n = NULL;
    patchterm[x][y][z] = &((*p)->n);

    tempx = x;
    tempy = y;
    tempz = z;

    return;
}


// feed()
// 
// finds local screens pixels to a photon collision and adds
// light to those pixels
//
void PIXELMACHINE::feed( const COLOR &c, const V &hit )
{
    static double dx = (double)PATCHDIM/(PX1-PX0);
    static double dy = (double)PATCHDIM/(PY1-PY0);
    static double dz = (double)PATCHDIM/(PZ1-PZ0);

    int i,j,k;
    int pixel;
    double m,n,o,dsq;

    if(hit.x<=PX0 || hit.x>=PX1 || hit.y<=PY0 || hit.y>=PY1 || hit.z<=PZ0 || hit.z>=PZ1) // not indexible
        return;

    int x = (int)((hit.x-PX0)*dx);
    int y = (int)((hit.y-PY0)*dy);
    int z = (int)((hit.z-PZ0)*dz);

    for(i=x-1; i<=x+1; i++) for(j=y-1; j<=y+1; j++) for(k=z-1; k<=z+1; k++)
    {
        if( i<0 || i>PATCHDIM-1 || j<0 || j>PATCHDIM-1 || k<0 || k>PATCHDIM-1 )
            continue;
        P *p = patch[i][j][k];
        while(p)
        {
            // test for proximity!
            m = p->v.x-hit.x;
            n = p->v.y-hit.y;
            o = p->v.z-hit.z;
            dsq = m*m+n*n+o*o;
            if( dsq <= PHOTONRADIUS )
            {
                pixel = p->i;
                dimg[pixel].r += c.r*(PHOTONRADIUS*1.5-dsq);
                dimg[pixel].g += c.g*(PHOTONRADIUS*1.5-dsq);
                dimg[pixel].b += c.b*(PHOTONRADIUS*1.5-dsq);
            }
            p = p->n;
        }
    }

    return;
}