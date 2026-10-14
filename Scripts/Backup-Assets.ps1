<#
.SYNOPSIS
    저장소에 들어가지 않는 Content 폴더를 외부 위치로 미러한다.

.DESCRIPTION
    이 프로젝트는 Fab 에셋을 재배포할 수 없어 Content 아래 여러 폴더를 git에서 제외한다
    (docs/known_issues.md B-02). 그 폴더에는 다운로드한 원본만 있는 것이 아니라
    그 위에 직접 한 작업 - 콜리전 형상, Nanite 설정, 머티리얼 인스턴스 - 이 섞여 있고,
    그 작업은 어디에도 버전 관리되지 않는다. 팩은 Fab에서 다시 받을 수 있지만
    메쉬 애셋 안에 넣은 콜리전은 다시 받을 수 없다.

    robocopy 미러이므로 첫 실행만 전체를 복사하고 이후에는 바뀐 파일만 복사한다.

.PARAMETER Destination
    백업 루트. 이 아래에 Content\<폴더> 구조로 미러한다.
    생략하면 CYBERPUNK_BACKUP 환경 변수를 쓴다.

.PARAMETER IncludeLargePacks
    손대지 않은 대용량 팩(ParagonLtBelica 2GB, Pack_Bonus 449MB)까지 포함한다.
    기본값은 제외다 - 이들은 수정하지 않았고 Fab/Epic에서 다시 받을 수 있다.

.PARAMETER Force
    에디터가 실행 중이어도 묻지 않고 진행한다.

.EXAMPLE
    .\Scripts\Backup-Assets.ps1 -Destination E:\NeonDistrictBackup

.EXAMPLE
    $env:CYBERPUNK_BACKUP = 'E:\NeonDistrictBackup'
    .\Scripts\Backup-Assets.ps1 -IncludeLargePacks
#>
[CmdletBinding(SupportsShouldProcess)]
param(
    [string] $Destination = $env:CYBERPUNK_BACKUP,
    [switch] $IncludeLargePacks,
    [switch] $Force
)

$ErrorActionPreference = 'Stop'

# 직접 수정한 작업이 들어 있는 폴더. 콜리전과 Nanite 설정이 이 안에 있다.
$Folders = @(
    'Content\MyAssets'        # 건물/창문 메쉬(직접), Fab 프롭, 텍스처
    'Content\cyberdistrict'   # KB3D 프롭 - 10/14 콜리전 형상 17종 추가
    'Content\Fab'
    'Content\NeonGrid'
    'Content\EasyFog'
    'Content\SystemFonts'
)

# 손대지 않았고 다시 받을 수 있는 팩.
$LargePacks = @(
    'Content\ParagonLtBelica'
    'Content\Pack_Bonus'
)

if ($IncludeLargePacks) { $Folders += $LargePacks }

$ProjectRoot = Split-Path -Parent $PSScriptRoot

if ([string]::IsNullOrWhiteSpace($Destination)) {
    throw '백업 위치를 지정하세요: -Destination <경로> 또는 $env:CYBERPUNK_BACKUP'
}
if (-not (Test-Path -LiteralPath $Destination)) {
    throw "백업 위치가 없습니다: $Destination (외부 드라이브가 연결되어 있습니까?)"
}

# 에디터가 애셋을 쓰는 중이면 반쯤 쓰인 uasset을 복사할 수 있다.
$editor = Get-Process UnrealEditor -ErrorAction SilentlyContinue
if ($editor) {
    $n = ($editor | Measure-Object).Count
    Write-Warning "UnrealEditor가 ${n}개 실행 중입니다. 저장하지 않은 변경은 백업되지 않고, 쓰는 중인 파일은 손상된 채 복사될 수 있습니다."
    if (-not $Force -and [Environment]::UserInteractive -and -not $WhatIfPreference) {
        # 비대화형(스케줄러, CI, 에이전트)에서는 묻지 않고 진행한다. 물으면 예외가 난다.
        $answer = Read-Host '계속하려면 y 를 입력하세요'
        if ($answer -ne 'y') { Write-Host '중단했습니다.'; return }
    }
}

$started = Get-Date
$packNote = '대용량 팩 제외'
if ($IncludeLargePacks) { $packNote = '대용량 팩 포함' }

Write-Host "백업 위치 : $Destination"
Write-Host "대상 폴더 : $($Folders.Count) 개 ($packNote)"
Write-Host ''

$results = @()
$failed  = $false

foreach ($rel in $Folders) {
    $src = Join-Path $ProjectRoot $rel
    if (-not (Test-Path -LiteralPath $src)) {
        Write-Host ("{0,-28} 없음 - 건너뜀" -f $rel) -ForegroundColor DarkGray
        continue
    }
    $dst = Join-Path $Destination $rel

    # /MIR 는 원본에서 지워진 파일을 백업에서도 지운다. 미러이므로 의도된 동작이다.
    # /R:2 /W:2 로 잠긴 파일에 오래 매달리지 않게 한다.
    $rcArgs = @($src, $dst, '/MIR', '/R:2', '/W:2', '/NFL', '/NDL', '/NP', '/NJH', '/NJS', '/MT:8')
    if ($WhatIfPreference) { $rcArgs += '/L' }

    $out  = & robocopy @rcArgs
    $code = $LASTEXITCODE

    # robocopy 종료 코드는 비트 플래그다. 0=변화 없음, 1=복사됨, 2=추가 항목,
    # 4=불일치, 8 이상=실패. 8 미만은 모두 성공으로 본다.
    if ($code -ge 8) {
        Write-Host ("{0,-28} 실패 (robocopy {1})" -f $rel, $code) -ForegroundColor Red
        $out | Select-Object -Last 12 | ForEach-Object { Write-Host "    $_" }
        $failed = $true
        continue
    }

    $stat = Get-ChildItem -LiteralPath $src -Recurse -File -ErrorAction SilentlyContinue |
            Measure-Object -Property Length -Sum
    $mb = [math]::Round($stat.Sum / 1MB)
    $state = '동기화'
    if ($code -eq 0) { $state = '변화 없음' }
    Write-Host ("{0,-28} {1,6} 파일 {2,6} MB  {3}" -f $rel, $stat.Count, $mb, $state)
    $results += [pscustomobject]@{ Folder = $rel; Files = $stat.Count; MB = $mb }
}

# 애셋과 저장소는 짝이 맞아야 의미가 있다. 어느 커밋 시점의 애셋인지 함께 남긴다.
$commit = & git -C $ProjectRoot rev-parse --short HEAD 2>$null
$dirty  = (& git -C $ProjectRoot status --porcelain 2>$null | Measure-Object).Count
$elapsed = [math]::Round(((Get-Date) - $started).TotalSeconds)

if (-not $WhatIfPreference) {
    $commitLine = $commit
    if ($dirty -gt 0) { $commitLine = "$commit  (커밋되지 않은 변경 ${dirty}개)" }

    $lines = @(
        'Neon District 에셋 백업'
        "시각      : $($started.ToString('yyyy-MM-dd HH:mm:ss'))"
        "소요      : ${elapsed}초"
        "저장소    : $ProjectRoot"
        "커밋      : $commitLine"
        "대용량 팩 : $packNote"
        ''
        '폴더별:'
    )
    $lines += $results | ForEach-Object { "  {0,-28} {1,6} 파일 {2,6} MB" -f $_.Folder, $_.Files, $_.MB }

    $manifest = Join-Path $Destination 'backup-manifest.txt'
    $lines | Set-Content -LiteralPath $manifest -Encoding utf8
    Write-Host ''
    Write-Host "목록 기록 : $manifest"
}

Write-Host ''
if ($failed) {
    Write-Host '일부 폴더가 실패했습니다. 위 메시지를 확인하세요.' -ForegroundColor Red
    exit 1
}
$totalMb = ($results | Measure-Object -Property MB -Sum).Sum
Write-Host ("완료 - {0} 폴더, {1} MB, {2}초" -f $results.Count, $totalMb, $elapsed) -ForegroundColor Green
