<#
.SYNOPSIS
Compiles assets for NetcodeClient
.DESCRIPTION
Provide switches to compile less, otherwise a full recompilation will happen
.PARAMETER Textures
if supplied, textures will be compiled
.PARAMETER Fonts
if supplied, fonts will be compiled
.PARAMETER Models
if supplied, models will be compiled
.PARAMETER OnlyNew
a compilation will be skipped, if the destination file exists
#>
Param(
    [Parameter(ValuefromPipeline=$true,Mandatory=$false)][switch]$Textures,
    [Parameter(ValuefromPipeline=$true,Mandatory=$false)][switch]$Fonts,
    [Parameter(ValuefromPipeline=$true,Mandatory=$false)][switch]$Models,
    [Parameter(ValuefromPipeline=$true,Mandatory=$false)][switch]$OnlyNew
)

$compileAll = !$Textures.IsPresent -AND !$Fonts.IsPresent

function Compile-Texture-Into-DDS {

    param (
        [string]$sourceTex,
        [int]$mipLevels,
        [string]$outFormat
    )

    $item = Get-Item $sourceTex

    $relativePath = Resolve-Path -relative $item.FullName
    $relativeDir = Resolve-Path -relative $item.Directory.FullName
    
    if($relativeDir.StartsWith(".\")) {
        $relativeDir = $relativeDir.Substring(2);
    }

    $outDir = [IO.Path]::Combine("compiled", $relativeDir)
    $outFile = [IO.Path]::Combine($outDir, $item.BaseName + ".dds")

    if($OnlyNew.IsPresent) {
        $exists = Test-Path $outFile -PathType Leaf

        if($exists) {
            return;
        }
    }

    ..\Tools\dxtex\texconv.exe -f $outFormat -m $mipLevels -ft dds -y -o $outDir $relativePath

}

function Compile-Texture-Copy {
    param (
        [string]$sourceTex
    )

    $outPath = [IO.Path]::Combine("compiled", $sourceTex)

    if($OnlyNew.IsPresent) {
        $exists = Test-Path $outPath -PathType Leaf

        if($exists) {
            return
        }
    }

    Copy-Item $sourceTex -Destination $outPath
}

function Compile-SpriteFont {
    param(
        [string]$outname,
        [string]$fontFamily,
        [int]$fontSize,
        [string]$fontStyle
    )

    $outPath = [IO.Path]::Combine("compiled\fonts", $outname)

    if($OnlyNew.IsPresent) {
        $exists = Test-Path $outPath -PathType Leaf

        if($exists) {
            return
        }
    }

    ..\Tools\dxtex\MakeSpriteFont.exe /CharacterRegion:0x20-0x7F /FontSize:$fontSize /FontStyle:$fontStyle /NoPremultiply /Sharp /TextureFormat:Rgba32 $fontFamily $outPath
}

function Compile-Model {
    param(
        [string]$manifest,
        [string]$outputName
    )

    $outPath = [IO.Path]::Combine("compiled\models", $outputName)

    if($OnlyNew.IsPresent) {
        $exists = Test-Path $outPath -PathType Leaf

        if($exists) {
            return
        }
    }

    ..\Build\x64-Debug-clang\NetcodeAssetCompiler\NetcodeAssetCompiler.exe --manifest $manifest --output $outPath --overwrite
}

if($compileAll -OR $Textures.IsPresent) {
    Compile-Texture-Into-DDS "textures/scifi_panel/Scifi_Panel_1K_albedo.tif" 11 "BC7_UNORM"
    Compile-Texture-Into-DDS "textures/scifi_panel/Scifi_Panel_1K_normal.tif" 11 "BC6H_UF16"
    Compile-Texture-Into-DDS "textures/scifi_panel/Scifi_Panel_1K_ao.tif" 11 "BC4_UNORM"
    Compile-Texture-Into-DDS "textures/scifi_panel/Scifi_Panel_1K_height.tif" 11 "BC4_UNORM"
    Compile-Texture-Into-DDS "textures/scifi_panel/Scifi_Panel_1K_metallic.tif" 11 "BC4_UNORM"
    Compile-Texture-Into-DDS "textures/scifi_panel/Scifi_Panel_1K_roughness.tif" 11 "BC4_UNORM"

    Compile-Texture-Into-DDS "textures/brick_1/Wall_Stone3_3x3_1K_albedo.tif" 11 "BC7_UNORM"
    Compile-Texture-Into-DDS "textures/brick_1/Wall_Stone3_3x3_1K_normal.tif" 11 "BC6H_UF16"
    Compile-Texture-Into-DDS "textures/brick_1/Wall_Stone3_3x3_1K_height.tif" 11 "BC4_UNORM"
    Compile-Texture-Into-DDS "textures/brick_1/Wall_Stone3_3x3_1K_ao.tif" 11 "BC4_UNORM"
    Compile-Texture-Into-DDS "textures/brick_1/Wall_Stone3_3x3_1K_roughness.tif" 11 "BC4_UNORM"

    Compile-Texture-Into-DDS "textures/chest_01/chest_01_diff_2k.png" 11 "BC7_UNORM"
    Compile-Texture-Into-DDS "textures/chest_01/chest_01_nrm_2k.png" 11 "BC6H_UF16"
    Compile-Texture-Into-DDS "textures/chest_01/chest_01_metal_2k.png" 11 "BC4_UNORM"
    Compile-Texture-Into-DDS "textures/chest_01/chest_01_ao_2k.png" 11 "BC4_UNORM"
    Compile-Texture-Into-DDS "textures/chest_01/chest_01_rough_2k.png" 11 "BC4_UNORM"

    Compile-Texture-Into-DDS "textures/envmaps/cloudynoon.dds" 10 "BC7_UNORM"

    Compile-Texture-Copy "textures/ui/aenami_dreamer.jpg"
    Compile-Texture-Copy "textures/ui/loading_icon.png"
    Compile-Texture-Copy "textures/ui/warning_icon.png"
}

if($compileAll -OR $Fonts.IsPresent) {
    Compile-SpriteFont "titillium16.spritefont" "Titillium Web" 16 "Regular"
    Compile-SpriteFont "titillium18.spritefont" "Titillium Web" 18 "Regular"
    Compile-SpriteFont "titillium24.spritefont" "Titillium Web" 24 "Regular"
    Compile-SpriteFont "titillium48bold.spritefont" "Titillium Web" 48 "Bold"
}

if($compileAll -OR $Models.IsPresent) {
    Compile-Model "wall_manifest.json" "wall.ncasset"
    Compile-Model "ybot_manifest.json" "ybot.ncasset"
}
