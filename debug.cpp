#include "debug.h"	
	
void openLOG(){
    vel   = fopen(velLOG,"w");
    diver = fopen(divLOG,"w");
    press = fopen(pressLOG,"w");
    color = fopen(colorLOG,"w");
    vort  = fopen(vortLOG,"w");

    return ;
}    
   
void displayVelocity(float velocity[][DIM][2])
{
    if(vel == NULL){
	cout<<"Velocity Profile is : \n";
	for (int i=0;i<DIM;i++) 
	{
		for (int j=0;j<DIM;j++)
		{
			if(velocity[i][j][0] || velocity[i][j][1])
			cout <<" ("<<velocity[i][j][0]<<","<<velocity[i][j][1]<<") at ("<<i<<","<<j<<")\n";
		}
	}
    }
    else{
	fprintf(vel,"Velocity Profile is : \n");
	for (int i=0;i<DIM;i++)
	{
		for (int j=0;j<DIM;j++)
		{
			if(velocity[i][j][0] || velocity[i][j][1])
			fprintf(vel,"(%f,%f) at [%d][%d]\n",velocity[i][j][0],velocity[i][j][1],i,j);
		}
	}        
    }
    return ;
}

void displayVorticity(float vorticity[][DIM]){
    if(vort == NULL){
	cout<<"Vorticity Profile is : \n";
	for (int i=0;i<DIM;i++) 
	{
		for (int j=0;j<DIM;j++)
		{
			cout <<" ("<<vorticity[i][j]<<") at ("<<i<<","<<j<<")\n";
		}
	}
    }
    else{
	fprintf(vort,"Vorticity Profile is : \n");
	for (int i=0;i<DIM;i++)
	{
		for (int j=0;j<DIM;j++)
		{
			fprintf(vort,"(%f) at [%d][%d]\n",vorticity[i][j],i,j);
		}
	}        
    }
    return ;
}

void displayDivergence(float divergence[][DIM])
{
    if(diver == NULL){
	cout<<"Divergence of Velocity Profile is : \n";
	for (int i=0;i<DIM;i++)
	{
		for (int j=0;j<DIM;j++)
		{
			cout <<divergence[i][j] <<" at ("<<i<<","<<j<<")\n";
		}
	}
    }
    else{
	fprintf(diver,"Divergence Profile is : \n");
	for (int i=0;i<DIM;i++)
	{
		for (int j=0;j<DIM;j++)
		{
			fprintf(diver,"(%f) at [%d][%d]\n",divergence[i][j],i,j);
		}
	}        
    }
    return ;
}


void displayPressure(float pressure[][DIM])
{
    if(press == NULL){
	cout<<"Pressure Profile is : \n";
	for (int i=0;i<DIM;i++)
	{
		for (int j=0;j<DIM;j++)
		{
			cout <<pressure[i][j] <<" at ("<<i<<","<<j<<")\n";
		}
	}
    }
    else{
	fprintf(press,"Pressure Profile is : \n");
	for (int i=0;i<DIM;i++)
	{
		for (int j=0;j<DIM;j++)
		{
			fprintf(press,"(%f) at [%d][%d]\n",pressure[i][j],i,j);
		}
	}        
    }
    return ;

}

void displayColor(float dye[][DIM][4])
{
    if(color == NULL){
	cout<<"Color Profile is : \n";
	for (int i=0;i<DIM;i++)
	{
		for (int j=0;j<DIM;j++)
		{
			cout <<"("<<dye[i][j][0]<<","<<dye[i][j][1]<<","<<dye[i][j][2]<<","<<dye[i][j][3]<<")" <<" at ("<<i<<","<<j<<")\n";
		}
	}
    }
    else{
	fprintf(color,"Color Profile is : \n");
	for (int i=0;i<DIM;i++)
	{
		for (int j=0;j<DIM;j++)
		{
			fprintf(color,"(%f,%f,%f,%f) at [%d][%d]\n",dye[i][j][0],dye[i][j][1],dye[i][j][2],dye[i][j][3],i,j);
		}
	}        
    }
    return ;
}

void printDetails(){

    system("clear");
    cout << "\n" ;
    cout << "___________________________________2D - FLUID SIMULATION_______________________________\n" ;
    cout << "\n";
    cout << "The program uses some CG-Profiles/Contexts specific to NVIDIA GPU's.\nHence it might give incorrect results on other GPU's\n\n";
    cout << "Tested on Nvidia GeForce GT 600 series : Geforce GT 635M GPU\nAnd GeForce GT 500 series : GeForce GT 540M GPU\n\n";
    cout << "Interactions:-\n\n";
    cout << "Mouse:- \n" ;
    cout << "1)Left Click and Drag to add dye to the display\n" ;
    cout << "2)Right Click and Drag to add a force impulse to the fluid\n";
    cout << "Keys:-\n" ;
    cout << "0) Key 's' = Start the simulation. \n";
    cout << "1) Key 'r' = Change the color of the dye added by mouse to RED. \n" ; 
    cout << "2) Key 'g' = Change the color of the dye added by mouse to GREEN. \n" ;
    cout << "3) Key 'b' = Change the color of the dye added by mouse to BLUE. \n" ;
    cout << "4) Key 'm' = Toggle the dye injection by mouse Left Click\n";
    cout << "5) Key 'f' = Toggle the addition of a force impulse by mouse rightclick\n";
    cout << "4) Key 'ESC' = Exit the Program.\n\n\n";
    cout << "Submitted as CSL-781 Project:-\n1)Rishav Binayak Das\n2)Dushyant Behl\n";
    
    return ;
}
