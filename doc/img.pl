#!/usr/bin/perl

$frontpiece = <<FRONT;
<html><title>img Command Line Image Processor</title><link rel="stylesheet" type="text/css" href="rawprocdoc.css"><body>

<h2 id="img">img Command Line Image Processor</h2>

<p>img is a command line image processor, in the way of ImageMagick and G'MIC.  It simply
provides a command line interface to the gimage library, exposing the tools with commands 
that look like this:</p>

<pre>img DSG_0001.NEF gamma:2.2 blackwhitepoint resize:640,0 sharpen:1 DSG_0001.jpg</pre>

<p>which opens the NEF, applies gamma, auto-adjusted black/white point, resize preserving
aspect ratio, just a bit of sharpen, and saves to a JPEG.  If you compiled rawproc, you 
probably compiled gimage first; img is the same program as gimg from gimage.  In the 
Windows installers, you are queried regarding adding the path to img to the PATH variable;
if you intend to use img, that's a good idea.</p>

<p>img has acouple of batch tricks worth mentioning here:</p>

<ul>

<li>Wild card processing: The "*" character can be used in input and output file names to apply
the same processing to a batch of files:

<pre>img "*.NEF" gamma:2.2 blackwhitepoint "processed/*.jpg"</pre><br>

This command will process each of the .NEFs in the current directory and save a correspondingly-named 
JPEG in the processed/ directory.  Note the quotes, this keeps the shell from expanding the filenames itself.</li><br>
<li>Incremental processing: img will skip an input file if the corresponding output file already 
exists.  This is handy for processing a set of images, and later coming back with the same SD card
and processing the new images shot since the first run.  There's no command line switch, img just 
does that all the time.  So, if you want to reprocess an image, you have to delete or move the 
previously generated output image.</li><br>

</ul>

<p>As of rawproc 0.7, img reads the rawproc configuration file in the default locations, first from 
the current working directory of img, next from the application default.  This allows img tools to
use the configured parameters of the respective rawproc tools, as well as the input parameters.  Two 
particular input parameters have special meaning:
<ol>
<li>input.jpeg|tiff|png|raw.parameters: Applies the input parameters specified in the specified 
property.  For example, 

<pre>img "*.NEF:input.raw.parameters" ...</pre><br>

retrieves the parameter string at input.raw.parameters and uses them when each input image is opened.</li>
<li>input.raw.libraw: This one tells img to round up all the input.raw.libraw parameters, construct 
a parameter string from them, and use them to open each image.</li>
</ol>

These changes now allow img to construct output images that contain processing strings sufficient 
to allow rawproc to open-source the image and reconstruct it from the original image. The following 
workflow is now possible (and is how I now shoot raw for all my images, including family snapshots):

<ol>
<li>Shoot pictures in raw, all day</li>
<li>At the end of the day, make a directory suitably named for the viewable images, and a subdirectory
under it for the raw files</li>
<li>Copy the raw files to the raw file subdirectory</li>
<ll>Open a command shell, cd to the raw directory, and run the following command:
<pre>img "*.NEF:input.raw.libraw" colorspace:[convert to the desired working space] blackwhitepoint resize:800 sharpen:1 "../*.jpg"</pre>
This will produce viewable JPEGs in the parent directory, sized to 800px on their largest dimension, with 
a full processing chain in their metadata</li>
<li>At leisure, regard the JPEGs and select ones for re-processing if desired.  If one is selected, 
it can be opened by dragging it to a rawproc desktop shortcut; rawproc will find the processing chain 
and ask if the original (raw) file is to be opened and re-processed.  Say "yes", and the raw file is 
opened, and the img processing chain is re-applied.  You can now modify it at will for various 
needs, such as creative cropping or resize for a different viewing situation, and save to a different
filename, or overwrite the original img-produced JPEG</li>
</ol>

<h3>img Tools</h3>
FRONT

print "$frontpiece\n<ul>\n";

my @conflines;
my $file = $ARGV[0];
my @lines = `grep \"//img\" $file`;
foreach $line (@lines) {
	chomp $line;
	$line =~ s/\r//;
	$line =~ s/^\s+\/\/img //;
	push @conflines, $line;
}

@conf = sort @conflines;
print @conf;

print "</ul></body></html>\n";
