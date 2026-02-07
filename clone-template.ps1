# clone-template.ps1
# Usage: .\clone-template.ps1 -Name "NeonOsc"

param(
    [Parameter(Mandatory=$true)]
    [string]$Name
)

$workspaceRoot = "d:/l/neon-synth-factory"
$templatePath = "$workspaceRoot/neon-template"
$newProjectPath = "$workspaceRoot/$Name"

# Validate template exists
if (-not (Test-Path $templatePath)) {
    Write-Error "Template not found: $templatePath. Make sure neon-template folder exists in the workspace."
    exit 1
}

# Validate project name is valid (alphanumeric + no spaces)
if ($Name -notmatch '^[a-zA-Z][a-zA-Z0-9]*$') {
    Write-Error "Project name must be alphanumeric and start with a letter (e.g., 'NeonOsc')."
    exit 1
}

# Check if target folder already exists
if (Test-Path $newProjectPath) {
    Write-Error "Project folder already exists: $newProjectPath. Choose a different name."
    exit 1
}

Write-Host "Cloning neon-template to $Name..." -ForegroundColor Cyan

# Copy template
Copy-Item -Path $templatePath -Destination $newProjectPath -Recurse -Force

# Define search/replace pairs (case-sensitive)
$searchReplace = @(
    @{ Search = "NeonTemplate"; Replace = "$Name" },
    @{ Search = "neontemplate"; Replace = $Name.ToLower() },
    @{ Search = "NEONTEMPLATE"; Replace = $Name.ToUpper() }
)

# Update CMakeLists.txt
$cmakePath = "$newProjectPath/CMakeLists.txt"
$content = Get-Content $cmakePath -Raw
foreach ($pair in $searchReplace) {
    $content = $content -replace [regex]::Escape($pair.Search), $pair.Replace
}
Set-Content -Path $cmakePath -Value $content -Encoding UTF8
Write-Host "Updated CMakeLists.txt" -ForegroundColor Green

# Update PluginProcessor.h/cpp and PluginEditor.h/cpp
$sourceFiles = @(
    "$newProjectPath/source/PluginProcessor.h",
    "$newProjectPath/source/PluginProcessor.cpp",
    "$newProjectPath/source/PluginEditor.h",
    "$newProjectPath/source/PluginEditor.cpp"
)

foreach ($file in $sourceFiles) {
    if (Test-Path $file) {
        Write-Host "Updating $file..." -ForegroundColor Cyan
        $content = Get-Content $file -Raw
        foreach ($pair in $searchReplace) {
            $content = $content -replace [regex]::Escape($pair.Search), $pair.Replace
        }
        Set-Content -Path $file -Value $content -Encoding UTF8
    }
}

# Update plugin code and bundle ID in CMakeLists.txt (if not already done)
$cmakeContent = Get-Content $cmakePath -Raw

# Ensure PLUGIN_CODE is set to 4-letter uppercase
$pluginCode = ($Name -replace '[^a-zA-Z]', '') -replace '(.{4}).*', '$1' -replace '.', { $_.Value.ToUpper() }
if ($pluginCode.Length -lt 4) {
    $pluginCode += "X" * (4 - $pluginCode.Length)
}

$cmakeContent = $cmakeContent -replace 'PLUGIN_CODE [A-Z0-9]{4}', "PLUGIN_CODE $pluginCode"
$cmakeContent = $cmakeContent -replace 'BUNDLE_ID com\.neonaudio\.[a-zA-Z0-9]+', "BUNDLE_ID com.neonaudio.$($Name.ToLower())"

Set-Content -Path $cmakePath -Value $cmakeContent -Encoding UTF8
Write-Host "Updated PLUGIN_CODE to $pluginCode and BUNDLE_ID to com.neonaudio.$($Name.ToLower())" -ForegroundColor Green

# Success message
Write-Host "" -ForegroundColor Green
Write-Host "🎉 SUCCESS: New synth project created at:" -ForegroundColor Green
Write-Host "   $newProjectPath" -ForegroundColor Yellow
Write-Host "" -ForegroundColor Green
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "1. Open in VS Code: code $Name" -ForegroundColor White
Write-Host "2. Build with: .\build.ps1 -Project $Name -Config Release" -ForegroundColor White
Write-Host "3. Update ModulationTargets.h and add your DSP engines." -ForegroundColor White
Write-Host "4. Document your architecture in planning docs/0X_$($Name.ToLower()).md" -ForegroundColor White
Write-Host "" -ForegroundColor Green