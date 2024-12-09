/*
;-----------------------------------------------------------------------------
;
; Copyright (c) 2005 Conexant Systems, Inc.
; 4000 MacArthur Blvd., Newport Beach, CA 92660 USA.
; All Rights Reserved Worldwide.
; Information Contained Herein is Proprietary to Conexant Systems, Inc.
; 
;
;-----------------------------------------------------------------------------
;
; Regions.ini
; 
; File contains the list of countries/regions that should be included
; into the project.
; The less is number of countries/regions, the less is the code size.
; 
; Comment out a country/region that should NOT be included into 
; the project.
; Remove the comment from a country/region that SHOULD BE be 
; included into the project.
;
;-----------------------------------------------------------------------------
;-----------------------------------------------------------------------------
;
; The format of the file is:
; - The part of a line after the comment sign (;) is ignored
; - Empty lines (containing just spaces or tabs) are ignored
; - Spacing is free
; - Significant line should contain the T.35 code of a country/region 
;   in hexadecimal format (the 0x prefix or h suffix are permitted, 
;   but not recommended); 
;   the rest of that line is ignored and used to indicate a name of a 
;   country/region for convenience.
;
;-----------------------------------------------------------------------------

; T.35 code - Country/region name
*/

0xB5	// - USA

0x00	// - Japan
0x01	// - Albania
0x02	// - Algeria
0x03	// - American Samoa
0x04	// - Germany
0x05	// - Anguilla
0x06	// - Antigua and Barbuda
0x07	// - Argentina
0x08	// - Ascension
0x09	// - Australia
0x0A	// - Austria
0x0B	// - Bahamas
0x0C	// - Bahrain
0x0D	// - Bangladesh
0x0E	// - Barbados
0x0F	// - Belgium
0x10	// - Belize
0x11	// - Benin
0x12	// - Bermuda
0x13	// - Bhutan
0x14	// - Bolivia
0x15	// - Botswana
0x16	// - Brazil
0x17	// - British Antarctic Territory
0x18	// - British Indian Ocean Territory
0x19	// - British Virgin Islands
0x1A	// - Brunei Darussalam
0x1B	// - Bulgaria
0x1C	// - Myanmar
0x1D	// - Burundi
0x1E	// - Belarus
0x1F	// - Cameroon
0x20	// - Canada
0x21	// - Cape Verde
0x22	// - Cayman Islands
0x23	// - Central African Republic
0x24	// - Chad
0x25	// - Chile
0x26	// - China
0x27	// - Colombia
0x28	// - Comoros
0x29	// - Congo
0x2A	// - Cook Islands
0x2B	// - Costa Rica
0x2C	// - Cuba
0x2D	// - Cyprus
;2E - Czech Republic
;2F - Cambodia
30 - Democratic People's Republic of Korea
;31 - Denmark
32 - Djibouti
;33 - Dominican Republic
;34 - Dominica
;35 - Ecuador
36 - Egypt
;37 - El Salvador
;38 - Equatorial Guinea
;39 - Ethiopia
;3A - Falkland Islands
;3B - Fiji
;3C - Finland
3D - France
;3E - French Polynesia
;40 - Gabon
;41 - Gambia
;43 - Angola
;44 - Ghana
;45 - Gibraltar
;46 - Greece
;47 - Grenada
;48 - Guam
;49 - Guatemala
;4B - Guinea
;4C - Guinea-Bissau
;4D - Guyana
;4E - Haiti
;4F - Honduras
50 - Hong Kong S.A.R
;51 - Hungary
52 - Iceland
53 - India
54 - Indonesia
55 - Iran
56 - Iraq
;57 - Ireland
58 - Israel
59 - Italy
;5A - Cote d'Ivoire
;5B - Jamaica
5C - Afghanistan
5E - Jordan
;5F - Kenya
;60 - Kiribati
61 - Korea
62 - Kuwait
63 - Laos
64 - Lebanon
;65 - Lesotho
;66 - Liberia
67 - Libya
;68 - Liechtenstein
;69 - Luxembourg
;6A - Macao S.A.R
;6B - Madagascar
6C - Malaysia
;6D - Malawi
;6E - Maldives
;6F - Mali
;70 - Malta
71 - Mauritania
;72 - Mauritius
;73 - Mexico
;74 - Monaco
;75 - Mongolia
;76 - Montserrat
77 - Morocco
;78 - Mozambique
;79 - Nauru
;7A - Nepal
;7B - Netherlands
;7C - Netherlands Antilles
;7D - New Caledonia
;7E - New Zealand
;7F - Nicaragua
;80 - Niger
;81 - Nigeria
;82 - Norway
83 - Oman
84 - Pakistan
;85 - Panama
;86 - Papua New Guinea
;87 - Paraguay
;88 - Peru
89 - Philippines
;8A - Poland
;8B - Portugal
;8C - Puerto Rico
8D - Qatar
;8E - Romania
;8F - Rwanda
;90 - Saint Kitts and Nevis
;92 - Saint Helena and Ascension
;93 - Saint Lucia
;94 - San Marino
;96 - Sao Tome and Principe
;97 - Saint Vincent and The Grenadines
98 - Saudi Arabia
;99 - Senegal
;9A - Seychelles
;9B - Sierra Leone
9C - Singapore
;9D - Solomon Islands
9E - Somalia
;9F - South Africa
;A0 - Spain
;A1 - Sri Lanka
A2 - Sudan
;A3 - Suriname
;A4 - Swaziland
;A5 - Sweden
;A6 - Switzerland
A7 - Syria
;A8 - Tanzania
A9 - Thailand
;AA - Togo
;AB - Tonga
;AC - Trinidad and Tobago
AD - Tunisia
AE - Turkey
;AF - Turks and Caicos Islands
;B0 - Tuvalu
;B1 - Uganda
;B2 - Ukraine
B3 - United Arab Emirates
B4 - UK
;B6 - Burkina Faso
;B7 - Uruguay
;B8 - Russia
;B9 - Vanuatu
;BA - Vatican City
;BB - Venezuela
BC - Vietnam
;BD - Wallis and Futuna
;BE - Western Samoa
BF - Yemen
;C1 - Yugoslavia
;C2 - Zaire
;C3 - Zambia
;C4 - Zimbabwe
C5 - Armenia
C7 - Turkmenistan
C8 - Azerbaijan
C9 - Georgia
CA - Kyrgyzstan
;CB - INMARSAT (Atlantic West)
;CC - INMARSAT (Indian)
;CD - INMARSAT (Pacific)
;CE - INMARSAT (Atlantic East)
;CF - INMARSAT
;D0 - International Freephone Service
D1 - Tajikistan
D2 - Kazakhstan
;D3 - Marshall Islands
;D4 - Micronesia, Federated States of
;D5 - Tokelau
;D6 - Niue
;D7 - Palau
;D8 - Norfolk Island
;D9 - Christmas Island
;DA - Tinian Island
;DB - Rota Island
;DC - Saipan Island
;DD - Cocos Keeling Islands
;DE - Martinique
;DF - French Guiana
;E0 - French Antilles
;E1 - Guadeloupe
;E2 - Guantanamo Bay
E3 - Saint Pierre and Miquelon
;E4 - Macedonia
;E5 - Bosnia and Herzegovina
;E6 - East Timor
;E7 - Andorra
;E8 - Moldova
EB - Uzbekistan
;EC - Greenland
;ED - Faroe Islands
;EE - Aruba
EF - Eritrea
;F0 - Mayotte
;F1 - Namibia
;F2 - Reunion Island
;F3 - Ascension Island
;F4 - Diego Garcia
;F5 - US Virgin Island
;F7 - Lithuania
;F8 - Latvia
;F9 - Estonia
;FA - Croatia
;FB - Slovakia
;FC - Slovenia
FE - Taiwan
