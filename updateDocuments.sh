#!/bin/bash

clear

### Root name of paper
mainfname="invivoHistotripsy"

### Generate all the other names needed to generate pdf from tex files
texName="$mainfname.tex"
cptoName="./$texName"
cpfromName="./workingVersion/$texName"
bibtexName="$mainfname.aux"
dviName="$mainfname.dvi"

GREEN="\033[1;32m"
CYAN="\033[1;36m" # Light Cyan (change 1 to 0 for normal cyan)
NC='\033[0m' # No Color
echo -e "\n- ${GREEN}Updating File${NC}: \t${CYAN}$cpfromName${NC} \n"

cd ./workingVersion/


aspell --mode=tex -c --add-tex-command="citep op" --add-tex-command="author op" --add-tex-command="address op" --add-tex-command="bibliography op" --add-tex-command="begin{equation} op" --add-tex-command="multicolumn pppp" --add-tex-command="textrm op" --add-tex-command="emph op" --ignore=2 $texName

git add $texName

### This loop spell checks all files in the 'sections' folder and copies them out of the git repo
for filefile in $(find sections/*.tex -maxdepth 1 -type f )
	do
		ffgit="./$filefile"
		ffext="../$filefile"
		ffsln="../workingVersion/$filefile"
		aspell --mode=tex -c --add-tex-command="citep op" --add-tex-command="author op" --add-tex-command="address op" --add-tex-command="bibliography op" --add-tex-command="begin{equation} op" --add-tex-command="multicolumn pppp" --add-tex-command="textrm op" --add-tex-command="emph op" --ignore=2 $ffgit

	
		git add $ffgit
	
		### checks if file (-f) does not exist, makes symbolic link to working version if it doesn't
		if [ ! -f $ffext ]; then
			ln -s $ffsln $ffext
		fi

	done



echo -e "- ${GREEN}git Commit Message${NC}: \n"
read -p "    " desc
gitoutput=$(git -c color.ui=always commit -m "$desc")

echo "$gitoutput"

cd ../

### checks if file (-f) exists, makes symbolic link to working version if it doesn't
if [ -f $cptoName ]; then
	### checks to see if file is NOT a symbolic link (-L), if it isn't it deletes it and makes symlink
	if [ ! -L $cptoName ]; then
		rm $cptoName
		ln -s $cpfromName $cptoName
	fi
else
	ln -s $cpfromName $cptoName
fi

ltoutput=$(latex -interaction=nonstopmode $texName)

if [[ $ltoutput == *"!"* ]]; then
	echo "LaTeX Error Detected"
	read -p "Do you want to see the output of the 'latex' command? (Y/n):  " ltyes

	if [[ "$ltyes" != "n" ]]; then
		echo -e "\n $ltoutput \n"
	fi
else

	biboutput=$(bibtex $bibtexName)
	ltoutput=$(latex $texName)

	dvipdf $dviName

	echo ""
	read -p "Do you want to see the output of the 'latex' command? (y/N):  " ltyes

	if [[ "$ltyes" == "y" ]]; then
		echo -e "\n $ltoutput \n"
	else
		clear
		echo -e "\n- ${GREEN}Updated File${NC}: \t${CYAN}$cpfromName${NC} \n"
		echo -e "- ${GREEN}git Output${NC}: \n"
		echo -e "$gitoutput\n"
	fi
fi

##################################### NOTE #####################################
### if you want to look at an older version of the file you're editing, go into
### the workingVersion directory and use the following command:
###
### git show [commit id of version you want]:[name of the file you're working] > [name of the file to write the output to]
###
################################################################################

