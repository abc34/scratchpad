// -sp-context: browser
//save array of rows of strings as utf-8 text
function saveAs(dataArray,filename,type){var blob=new Blob(dataArray,{type:type}),a=document.createElement("a"),url=URL.createObjectURL(blob);a.href=url;a.download=filename;document.body.appendChild(a);a.click();setTimeout(()=>{document.body.removeChild(a);window.URL.revokeObjectURL(url);},0);}
var Files = [
	{name:"UnicodeData.txt"},
	{name:"SpecialCasing.txt"},
	{name:"CaseFolding.txt"},
	{name:"DerivedCoreProperties.txt"}
	];


function parse1()
{
	//1. UnicodeData.txt
	var rows=Files[0].data.split("\n");while(!rows[rows.length-1])rows.pop();
	var L1=rows.map(x=>x.split(";"));
	console.log("<check> L1 is %o",L1.every(x=>x.length===15));
	//преобразование в число и вычисление приращения для полей [12],[13],[14]==toUpper,toLower,toTitle
	L1.every(x=>
	{
		x[0]=Number.parseInt(x[0],16);
		if(x[3])x[3]=Number.parseInt(x[3],10);
		if(x[9])x[9]=x[9]==="Y";
		if(x[12] || x[13] || x[14])
		{
			x[12]=x[12]?Number.parseInt(x[12],16)-x[0]:0;
			x[13]=x[13]?Number.parseInt(x[13],16)-x[0]:0;
			x[14]=x[14]?Number.parseInt(x[14],16)-x[0]:0;
		}
		return true;
	});
	console.log("<info> Not empties [5]-decomposition =",L1.filter(x=>!!x[5]).length);
	//exclude [1] end with "First>" or "Last>"
	var L1_excluded=[];
	L1=L1.filter(x=>{if(x[1].endsWith(" First>") || x[1].endsWith(" Last>")){L1_excluded.push(x);return false};return true;});
	console.log("<info> L1",L1,L1.length);
	console.log("<info> L1 excluded",L1_excluded,L1_excluded.length);
	return L1;
}
function parse3()
{
	//3.CaseFolding.txt
	var L3, rows=Files[2].data.split("\n");while(!rows[rows.length-1])rows.pop();
	rows=rows.filter(x=>(x.startsWith("#")||x==="")===false);
	L3=rows.map(x=>x.split("; "));
	console.log("<check> L3 is %o",L3.every(x=>x.length===4));
	//exclude x[1]="T" (default behaviour) and convert to integer
	var L3_excluded = L3.filter(x=>x[1]==="T" || x[1]==="F");
	L3=L3.filter(x=>{if(x[1]==="T" || x[1]==="F")return false;
									 x[0]=Number.parseInt(x[0],16);
									 x[2]=Number.parseInt(x[2],16)-x[0];
									 return true;});
	console.log("<info> L3",L3,L3.length);
	console.log("<info> L3 excluded",L3_excluded,L3_excluded.length);
	return L3;
}
function parse4()
{
	//4.DerivedCoreProperties.txt
	//Math, Alphabetic, Lowercase, Uppercase, Cased, Case_Ignorable,
	//Changes_When_Lowercased, Changes_When_Uppercased, Changes_When_Titlecased,
	//Changes_When_Casefolded, Changes_When_Casemapped, ID_Continue, ID_Start,
	//XID_Start, XID_Continue, Default_Ignorable_Code_Point, Grapheme_Extend,
	//Grapheme_Base, Grapheme_Link
	var L4, rows=Files[3].data.split("\n");while(!rows[rows.length-1])rows.pop();
	rows=rows.filter(x=>(x.startsWith("#")||x==="")===false);
	L4=rows.map(x=>x.split("; "));
	console.log("<check> L4 is %o",L4.every(x=>x.length===2));
	//exclude
	var pattern=/^ID_Start|^ID_Continue/,
			L4_excluded = L4.filter(x=>x[1].search(pattern)===-1);
	L4=L4.filter(x=>{if(x[1].search(pattern)===-1)return false;
						x[0]=x[0].split("..");
						x[0][0]=Number.parseInt(x[0][0],16);
						if(x[0].length===1)x[0][1]=1;else x[0][1]=Number.parseInt(x[0][1],16)-x[0][0]+1;
						var c=x[1].split(" #");
						x[1]=c[0];x[2]=c[1];
						return true;});
	console.log("<info> L4",L4,L4.length);
	console.log("<info> L4 excluded",L4_excluded,L4_excluded.length);
	return L4;
}





function SelectHash(source)
{
	var N=3031,//source.length+500,
			p=16769023;//1046527
	var gcd=function(a,b){var r=a;if(a<b){a=b;b=r;}while(1){r=a%b;if(r===0)return b;if(r===1)return 1;a=b;b=r;}};
	while(gcd(N,2*3*5*7*11*13)===1)N+=2;
	function hash(x,C)
	{
		x[0]=(x[0]*C+0x29)^774638;//x[0]%=N;
		x[0]=(x[0]*C+0x33);//x[0]%=N;
		x[0]=(x[0]*C+0x2)^179613;//x[0]%=N;
		x[0]=(x[0]*C+0x1);//x[0]%=N;
		x[0]=(x[0]*C)%p;
		x[0]%=N;
	}
	var a=new Uint8Array(N), ui=new Uint32Array(1),min=[0];
	for(var i=0;i<10*1e6;i++)
	{
		a.fill(0);
		if(source.every(x=>{ui[0]=x;hash(ui,i);a[ui[0]]++;return a[ui[0]]<3;}))
		{
			var count=a.reduce((r,x)=>{if(x===2)r++;return r;},0);
			if(count<10)console.log("C",i,"a",a,count);
			return;
		}
		else if(!(i&0xFFFF))console.log("i",i,"a",a,a.indexOf(3));
	}
	console.log("C not found");
}









function Do()
{
	
  //UnicodeData.txt fields:
	//0-code,1-name,        2-category,            3-combining_class(0-255 or empty),
	//4-bidi_class,         5-decomp_type(decomp_mapping),
	//6-decimal_value,      7-digit_value,         8-numeric_value,
	//9-bidi_mirrored(Y|N), 10-old_name,          11-comment,
  //12-uppercase_mapping, 13-lowercase_mapping, 14-titlecase_mapping
	
	//general category:
	//	Lu =  1, Ll =  2, Lt =  3, Lm =  4, Lo =  5,
	//	Mn =  6, Mc =  7, Me =  8,
	//	Nd =  9, Nl =  10, No =  11,
	//	Pc =  12, Pd =  13, Ps =  14, Pe =  15, Pi =  16, Pf =  17, Po =  18,
	//	Sm =  19, Sc =  20, Sk =  21, So =  22,
	//	Zs =  23, Zl =  24, Zp =  25,
	//	Cc =  26, Cf =  27, Cs =  28, Co =  29, Cn =  30;

	//1.UnicodeData.txt
	//3.CaseFolding.txt
	L=parse1();
	L3=parse3();
	L4=parse4();
	
	//L4
	//Math, Alphabetic, Lowercase, Uppercase, Cased, Case_Ignorable,
	//Changes_When_Lowercased, Changes_When_Uppercased, Changes_When_Titlecased,
	//Changes_When_Casefolded, Changes_When_Casemapped, ID_Continue, ID_Start,
	//XID_Start, XID_Continue, Default_Ignorable_Code_Point, Grapheme_Extend,
	//Grapheme_Base, Grapheme_Link
	
	
	
	
	
	
	
	
	
	
	L1=L;//store full source to L1

	//--------------------------------------------------
	//добавим simple case folding в [15]
	//--------------------------------------------------
	console.log("  Add simple fold case field to [15]");
	L1.every(x=>{x[15]="";return true;});
	L3.every(x=>
	{
		if(L1.every(y=>{if(y[0]!==x[0])return true;y[15]=x[2];return false;}))
		{
			console.log("<error> codePoint=%o not found in L1",x[0]);
		}
		return true;
	});


	//Add flags ID_Start, ID_Continue, Alphabetic
	//console.log("  Add flags ID_Start, ID_Continue, Alphabetic field to [16]");
	//L1.every(x=>{x[16]="";return true;});
	//L4.every(x=>
	//{
	//	if(L1.every(y=>{if(y[0]<x[0][0] || y[0]>=x[0][0]+x[0][1])return true;y[16]+="|"+x[1];return false;}))
	//	{
	//		console.log("<error> codePoint=%o not found in L1",[x[0],x[1]]);
	//	}
	//	return true;
	//});

	
	
	
	
	//--------------------------------------------------
	//выделим строки с информацией об Lu,Ll,Lt
	//--------------------------------------------------
	//отбрасываем строки с пустыми полями toUpper,toLower,toTitle, foldCase
	L1=L1.filter((x)=>x[12] || x[13] || x[14] || x[15]);
	L1=L1.map(x=>{if(x[15]==="")x[15]=0;return x;});
	console.log("<info> L1 simple cased",L1,L1.length);
	console.log("<info> max abs fc=%o",L1.reduce((r,x)=>x[13]>0?r:Math.max(Math.abs(x[15]||0),r),0))
	//return;

	
	//search hash
	//SelectHash(L1.map(x=>x[0]));return;

	
	
	
	console.log("<info> Number of rows [toUpper]!==[toTitle]",L1.filter(x=>x[12]!==x[14]).length);
	console.log("<info> Number of rows L? have valid Lu,Ll,Lt",L.filter(x=>x[2].startsWith("L") && (x[12]||x[13]||x[14])).length);
	
	//вычислим все различные Lu,Ll,Lt,foldCase
	//сохраним ссылку в L[.][1]=i => R[i]
	d1t=[[0,0,0,0]];
	Ri=L1.reduce((r,x)=>
	{
		var c=[x[12],x[13],x[14],x[15]].toString(),i=r.indexOf(c);
		if(i==-1){r.push(c);i=r.length-1;d1t.push([x[12],x[13],x[14],x[15]]);}
		x[1]=i;return r;
	},["empty"]);
	console.log(Ri);
	

	//таблицы для вычисления isCased,toUpper,toLower,toTitle,foldCase
	R1={a1max:0,h1max:0,a1t:[],rsh:7,msk:127};
	R1=L1.reduce((r,x)=>{var c=x[0]>>R1.rsh;if(!r.a1t.includes(c)){r.a1t.push(c);}return r;},R1);R1.a1max=R1.a1t.length;R1.h1max=R1.a1t[R1.a1max-1];
	R1.h1t=new Int8Array(R1.h1max+1).fill(-1);
	if(R1.a1t.length>127){console.log("\n<error> Cell type in R1.h1t is int8, but values overflow it!");return;}
	R1=R1.a1t.reduce((r,x,i)=>{r.h1t[x]=i;return r;},R1);
	R1.a1t=new Array(R1.a1max).fill(0);
	R1=R1.a1t.reduce((r,x,i)=>{r.a1t[i]=new Uint8Array(1<<R1.rsh);return r;},R1);
	R1=L1.reduce((r,x)=>{var h=x[0]>>R1.rsh,c=x[0]&R1.msk;r.a1t[r.h1t[h]][c]=x[1];return r;},R1);
	//test
	console.log("<test> a1t is %o",L1.every((x)=>{return R1.a1t[R1.h1t[x[0]>>R1.rsh]][x[0]&R1.msk]===x[1];}));
	R1.d1t=d1t;
	R1.d1max=R1.d1t.reduce((r,x)=>{if(r<Math.abs(x[0]))r=Math.abs(x[0]);if(r<Math.abs(x[1]))r=Math.abs(x[1]);if(r<Math.abs(x[2]))r=Math.abs(x[2]);return r;},0);
	R1.isCased=function(codePoint)
	{
		var h=codePoint>>R1.rsh;
		return h<=R1.h1max && R1.h1t[h]>=0 && R1.a1t[R1.h1t[h]][codePoint&R1.msk]>0;
	};
	//testing index i=x[1] => Ri[i]
	console.log("<test> d1t mapping is %o",L1.every((x)=>{return Ri[x[1]]===[x[12],x[13],x[14],x[15]].toString();}));
	console.log("<test> is d1t has Int16 values = %o",(R1.d1max<=0x7FFF));
	console.log("<test> is d1t.length < 256     = %o",(R1.d1t.length<=0xFF));
	console.log("<test> isCased is %o",L1.every(x=>{return R1.isCased(x[0]);}));
	c=0x10400-0;console.log("<test> c =%o isCased = %o",c.toString(16),(true===R1.isCased(c)));
	console.log("<info> R1",R1);
	console.log("<info> d1t.length = %o, has el = [%o x Int32],   total = %o bytes",R1.d1t.length,R1.d1t[0].length,R1.d1t.length*R1.d1t[0].length*4);
	console.log("<info> h1t.length = %o, has el = [Int8],          total = %o bytes",R1.h1t.length,R1.h1t.length);
	console.log("<info> a1t.length = %o, has el = [%o x Uint8],    total = %o bytes",R1.a1t.length,1<<R1.rsh,R1.a1t.length*(1<<R1.rsh));
	console.log("<info>                                               total = %o bytes",R1.a1t.length*(1<<R1.rsh)+R1.h1t.length+R1.d1t.length*R1.d1t[0].length*4);
	
	//debugger;
	//print as c code
	var code = [];
	code.push("struct CharInfo{int32_t toUpper,toLower,toTitle,foldCase;};\n");
	code.push("struct Chidx{uint8_t idx["+(1<<R1.rsh)+"];};\n");
	code.push("CharInfo d1t["+R1.d1t.length+"] = {");
	R1.d1t.reduce((r,x)=>{r.push("{"+x.join()+"},");return r;},code);
	code.push("};\n");
	code.push("int8_t h1t["+R1.h1t.length+"] = {");code.push(R1.h1t.join());code.push("};\n");
	code.push("Chidx a1t["+R1.a1t.length+"] = {\n");
	R1.a1t.reduce((r,x)=>{r.push("{ "+x.join()+" },\n");return r;},code);
	code.push("};\n");
	code.push("bool isCased(uint32_t codePoint)\n{\n	uint32_t h = codePoint >> "+R1.rsh+";\n	return h < "+(R1.h1max+1)+" && h1t[h] >= 0 && a1t[h1t[h]].idx[codePoint & "+R1.msk+"]>0;\n};\n");
	code.push("uint32_t toUpper(uint32_t codePoint)\n{\n	uint32_t h = codePoint >> "+R1.rsh+";\n	if(h < "+(R1.h1max+1)+" && h1t[h] >= 0)\n		codePoint += d1t[a1t[h1t[h]].idx[codePoint & "+R1.msk+"]].toUpper;\n	return codePoint;\n};\n");
	code.push("uint32_t toLower(uint32_t codePoint)\n{\n	uint32_t h = codePoint >> "+R1.rsh+";\n	if(h < "+(R1.h1max+1)+" && h1t[h] >= 0)\n		codePoint += d1t[a1t[h1t[h]].idx[codePoint & "+R1.msk+"]].toLower;\n	return codePoint;\n};\n");
	code.push("uint32_t toTitle(uint32_t codePoint)\n{\n	uint32_t h = codePoint >> "+R1.rsh+";\n	if(h < "+(R1.h1max+1)+" && h1t[h] >= 0)\n		codePoint += d1t[a1t[h1t[h]].idx[codePoint & "+R1.msk+"]].toTitle;\n	return codePoint;\n};\n");
	code.push("uint32_t foldCase(uint32_t codePoint)\n{\n	uint32_t h = codePoint >> "+R1.rsh+";\n	if(h < "+(R1.h1max+1)+" && h1t[h] >= 0)\n		codePoint += d1t[a1t[h1t[h]].idx[codePoint & "+R1.msk+"]].foldCase;\n	return codePoint;\n};\n");
	
	//code = code.join("");	
	//saveAs(code,"code_tocase.txt","application/octet-binary");

	
	console.log("h1t",
	R1.h1t.reduce((r,x,i)=>{var k=(i*13+1)%47;if(!r[k] && x>=0)r[k]={};if(x>=0)r[k][i]=x;return r;},{})
		)
	var r=[];
	for(var C=0;C<100;C++)
	{
		r.push(R1.h1t.reduce((r,x,i)=>{var k=(i*C+0)%47;if(!r.hasOwnProperty(k))r[k]={};if(x>=0)r[k][i]=x;return r;},[])
					 .reduce((r,x)=>{if(Object.keys(x).length>0)r++;return r;},0));
	}
	console.log(r);
	
	//full reverse index
	//S=L.map(x=>x[0]);console.log("S",S,"Inv",S.reduce((r,x,i)=>{r[x]=i;return r;},new Array(0x110000)));
	
	
	//TODO: нужно добавить caseFold поле [15] в L
	//и скопировать все C+S+F в это поле
	//затем снова создать L1 with case folded

	//L.reduce((r,x)=>{var h=x[0]>>4;if(!r.a.includes(h)){r.a.push(h);r.b.push(new Array(16));}r.b[r.b.length-1][x[0]&15]=1;return r;},{a:[],b:[]})
};






//start loading
Promise.all(Files.map(x=>fetch(x.name)))
	.then(r=>Promise.all(r.map(x=>x.text())))
	.then(r=>Files.every((x,i)=>x.data=r[i]))
	.then(r=>setTimeout(Do,1))
	.catch(e=>console.log("<Error> Need check file names !"));
