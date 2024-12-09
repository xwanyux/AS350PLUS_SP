
#ifndef __ECL_TAG_STRUCTURE_H__
#define __ECL_TAG_STRUCTURE_H__

//	Tag Structure
struct ecl_tag{
	unsigned char	*Length;
	unsigned char	*Value;
}__attribute__((packed));;	
typedef struct ecl_tag ECL_TAG;

//	Tag Table
struct ecl_tagtable{
	unsigned char	Tag[4];				//Tag
	unsigned char	Format;				//Format
	unsigned char	MASTER_MinLength[2];//Minimum Length of MasterCard
	unsigned char	MASTER_MaxLength[2];//Maximum Length of MasterCard
	unsigned char	MASTER_IsKnown;		//IsKnown of MasterCard
	unsigned char	MASTER_UC;			//Update Condition of MasterCard
	unsigned char	MASTER_Template;	//Template of MasterCard
	unsigned char	VISA_Length;		//Length of VISA
	unsigned char	VISA_UC;			//Update Capability of VISA
	unsigned char	VISA_IU;			//Issuer Update of VISA
	unsigned char	VISA_R;				//Retrieval of VISA
	unsigned char	JCB_Length;			//Tag Length of JCB
	unsigned char	JCB_Presence;		//Presence of JCB
	unsigned char	JCB_Length_Def;		//Var or Fixed length of JCB	0 : Undefined or length is large than 256 , 1 : Fixed, 2 : Var in variable length
	unsigned char	AE_IsKnown;			//Tag Is Known for AE
	unsigned char	DPS_IsKnown;		//IsKnown of D-PAS
	unsigned char	UPI_Length[2];		//Length of UPI
}__attribute__((packed));;	
typedef struct ecl_tagtable ECL_TAGTABLE;
#define ECL_SIZE_TAGTABLE	sizeof(ECL_TAGTABLE)


#endif


