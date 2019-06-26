//
// Cretaed by Ala Altaweel
//

#include "gns/GnsServiceClient.h"
using namespace hrp;
using namespace std;


/**
 * These functions can be used by RSock
 * @param argc
 * @param argv
 * @return
 */

int main(int argc , char *argv[])
{

	GnsServiceClient gns;

	//cout<<"updateIP "<<gns.updateIP()<<endl;

	string ownAccountName = gns.getOwnAccountName();
	cout<<"Own account Name "<< ownAccountName <<endl;
  	

	
	string ownGUID =  gns.getOwnGUID();
	cout<<"Own GUID " << ownGUID<<endl;

	cout<<"getGUIDbyAccountName " <<gns.getGUIDbyAccountName(ownAccountName)<<endl;
	cout<<"getAccountNamebyGUID "<<gns.getAccountNamebyGUID(ownGUID)<<endl;

	cout<<"getIPbyGUID: ";
	vector<string> ownIps = gns.getIPbyGUID(ownGUID);
	gns.print_vector(ownIps);

	cout<<"\ngetIPbyAccountName: ";
	gns.print_vector(gns.getIPbyAccountName(ownAccountName));

	cout<<"\ngetGUIDbyIP: ";
	gns.print_vector(gns.getGUIDbyIP(ownIps[0]));

	cout<<"\ngetAccountNamebyIP: ";
	gns.print_vector(gns.getAccountNamebyIP(ownIps[0]));
	

	return 0;
}
