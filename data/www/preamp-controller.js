class PreampController extends HTMLElement {
  constructor() {
    super();
    this.sysStatus = {};
    this.currentPage = 'main';
    this.eventSource = null;
  }

  connectedCallback() {
    this.initializeTheme(); // Apply theme FIRST
    this.render();
    this.updateToggleState(); // Update toggle after render
    this.setupEventListeners();
    this.handleInitialRoute(); // Handle hash on page load
    this.loadSystemStatus();
    this.connectEventSource();
  }

  disconnectedCallback() {
    if (this.eventSource) {
      this.eventSource.close();
    }
  }

  render() {
    this.innerHTML = `
      <!-- Header -->
      <div class="row header">
        <div class="col-xs-12 col-lg-4 col-lg-offset-4 center-xs middle-xs">
          <a href="#" id="home-btn" class="header-home-link">
            <div class="row middle-xs">
            <div class="hidden-xs col-md-4 first-md">
              <figure>
                <svg xmlns="http://www.w3.org/2000/svg" style="isolation:isolate" viewBox="0 0 54 11.482" width="54pt" height="11.482pt">
                  <path d="M 0.443 0 L 0.443 0 L 10.875 0 L 10.875 0 Q 11.056 0 11.187 0.131 L 11.187 0.131 L 11.187 0.131 Q 11.318 0.262 11.318 0.443 L 11.318 0.443 L 11.318 2.936 L 11.318 2.936 Q 11.318 3.117 11.187 3.248 L 11.187 3.248 L 11.187 3.248 Q 11.056 3.379 10.875 3.379 L 10.875 3.379 L 4.429 3.379 L 4.429 4.429 L 9.809 4.429 L 9.809 4.429 Q 9.99 4.429 10.121 4.56 L 10.121 4.56 L 10.121 4.56 Q 10.252 4.691 10.252 4.872 L 10.252 4.872 L 10.252 7.349 L 10.252 7.349 Q 10.252 7.529 10.121 7.66 L 10.121 7.66 L 10.121 7.66 Q 9.99 7.792 9.809 7.792 L 9.809 7.792 L 4.429 7.792 L 4.429 11.039 L 4.429 11.039 Q 4.429 11.22 4.298 11.351 L 4.298 11.351 L 4.298 11.351 Q 4.166 11.482 3.986 11.482 L 3.986 11.482 L 0.443 11.482 L 0.443 11.482 Q 0.262 11.482 0.131 11.351 L 0.131 11.351 L 0.131 11.351 Q 0 11.22 0 11.039 L 0 11.039 L 0 0.443 L 0 0.443 Q 0 0.262 0.131 0.131 L 0.131 0.131 L 0.131 0.131 Q 0.262 0 0.443 0 Z M 16.174 11.482 L 16.174 11.482 L 13.074 11.482 L 13.074 11.482 Q 12.926 11.482 12.819 11.376 L 12.819 11.376 L 12.819 11.376 Q 12.713 11.269 12.713 11.171 L 12.713 11.171 L 12.713 11.171 Q 12.713 11.072 12.729 11.023 L 12.729 11.023 L 16.305 0.541 L 16.305 0.541 Q 16.453 0 17.043 0 L 17.043 0 L 21.603 0 L 21.603 0 Q 22.194 0 22.341 0.541 L 22.341 0.541 L 25.917 11.023 L 25.917 11.023 Q 25.934 11.072 25.934 11.171 L 25.934 11.171 L 25.934 11.171 Q 25.934 11.269 25.827 11.376 L 25.827 11.376 L 25.827 11.376 Q 25.721 11.482 25.573 11.482 L 25.573 11.482 L 22.473 11.482 L 22.473 11.482 Q 21.948 11.482 21.735 10.925 L 21.735 10.925 Q 19.622 4.177 19.323 3.166 Q 19.024 4.203 16.912 10.925 L 16.912 10.925 Q 16.699 11.482 16.174 11.482 Z M 32.971 0.443 L 32.971 8.12 L 38.564 8.12 L 38.564 8.12 Q 38.745 8.12 38.876 8.251 L 38.876 8.251 L 38.876 8.251 Q 39.007 8.382 39.007 8.563 L 39.007 8.563 L 39.007 11.039 L 39.007 11.039 Q 39.007 11.22 38.876 11.351 L 38.876 11.351 L 38.876 11.351 Q 38.745 11.482 38.564 11.482 L 38.564 11.482 L 28.985 11.482 L 28.985 11.482 Q 28.804 11.482 28.673 11.351 L 28.673 11.351 L 28.673 11.351 Q 28.542 11.22 28.542 11.039 L 28.542 11.039 L 28.542 0.443 L 28.542 0.443 Q 28.542 0.262 28.673 0.131 L 28.673 0.131 L 28.673 0.131 Q 28.804 0 28.985 0 L 28.985 0 L 32.528 0 L 32.528 0 Q 32.708 0 32.84 0.131 L 32.84 0.131 L 32.84 0.131 Q 32.971 0.262 32.971 0.443 L 32.971 0.443 L 32.971 0.443 Z M 49.505 5.331 L 53.934 10.908 L 53.934 10.908 Q 54 11.007 54 11.138 L 54 11.138 L 54 11.138 Q 54 11.269 53.893 11.376 L 53.893 11.376 L 53.893 11.376 Q 53.787 11.482 53.639 11.482 L 53.639 11.482 L 49.407 11.482 L 49.407 11.482 Q 49.03 11.482 48.817 11.22 L 48.817 11.22 L 45.962 7.693 L 45.962 11.039 L 45.962 11.039 Q 45.962 11.20 45.831 11.351 L 45.831 11.351 L 45.831 11.351 Q 45.7 11.482 45.519 11.482 L 45.519 11.482 L 42.14 11.482 L 42.14 11.482 Q 41.96 11.482 41.829 11.351 L 41.829 11.351 L 41.829 11.351 Q 41.697 11.22 41.697 11.039 L 41.697 11.039 L 41.697 0.443 L 41.697 0.443 Q 41.697 0.262 41.829 0.131 L 41.829 0.131 L 41.829 0.131 Q 41.96 0 42.14 0 L 42.14 0 L 45.519 0 L 45.519 0 Q 45.7 0 45.831 0.131 L 45.831 0.131 L 45.831 0.131 Q 45.962 0.262 45.962 0.443 L 45.962 0.443 L 45.962 3.527 L 48.554 0.279 L 48.554 0.279 Q 48.8 0 49.161 0 L 49.161 0 L 53.114 0 L 53.114 0 Q 53.262 0 53.368 0.107 L 53.368 0.107 L 53.368 0.107 Q 53.475 0.213 53.475 0.353 L 53.475 0.353 L 53.475 0.353 Q 53.475 0.492 53.393 0.591 L 53.393 0.591 L 49.505 5.331 L 49.505 5.331 Z" fill="rgb(255,255,255)"/>
                </svg>
              </figure>
            </div>
            <div class="col-md-4 col-xs-4">
              <figure class="logo">
                <svg xmlns="http://www.w3.org/2000/svg" style="isolation:isolate" viewBox="0 0 128 128">
                  <path d="M 0 64 C 0 28.676 28.676 0 64.003 0 C 99.324 0 128 28.676 128 64 C 128 99.321 99.324 128 64.003 128 C 28.676 128 0 99.321 0 64 Z M 48.332 92.56 L 48.332 92.56 L 32.909 92.56 L 32.909 92.56 Q 32.177 92.56 31.642 92.031 L 31.642 92.031 L 31.642 92.031 Q 31.114 91.5 31.114 91.009 L 31.114 91.009 L 31.114 91.009 Q 31.114 90.519 31.196 90.277 L 31.196 90.277 L 48.986 38.132 L 48.986 38.132 Q 49.719 35.44 52.656 35.44 L 52.656 35.44 L 75.344 35.44 L 75.344 35.44 Q 78.281 35.44 79.014 38.132 L 79.014 38.132 L 96.804 90.277 L 96.804 90.277 Q 96.889 90.519 96.889 91.009 L 96.889 91.009 L 96.889 91.009 Q 96.889 91.5 96.358 92.031 L 96.358 92.031 L 96.358 92.031 Q 95.823 92.56 95.091 92.56 L 95.091 92.56 L 79.668 92.56 L 79.668 92.56 Q 77.058 92.56 75.998 89.786 L 75.998 89.786 Q 65.486 56.217 64.003 51.19 Q 62.511 56.346 52.005 89.786 L 52.005 89.786 Q 50.942 92.56 48.332 92.56 L 48.332 92.56 L 48.332 92.56 Z" fill-rule="evenodd" fill="rgb(255,255,255)"/>
                </svg>
              </figure>
            </div>
            <div class="col-md-4 col-xs-4 first-xs last-md">
              <p>Pre</p>
            </div>
            </div>
          </a>
        </div>
        
        <!-- Header Controls -->
        <div id="theme-toggle">
          <button id="dark-mode-toggle" class="theme-button">
            <!-- Icon will be inserted here by JavaScript -->
          </button>
        </div>
        <div id="settings-icon">
          <a href="#" class="white-halo" id="settings-btn">
            <svg class="icon icon-white" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24">
              <circle cx="12" cy="12" r="3"></circle>
              <path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"></path>
            </svg>
          </a>
        </div>
      </div>

      <!-- Main Content -->
      <div id="app">
        <div id="page-main" class="page active">
          <!-- Main Volume/Input Page -->
          <div class="row">
            <div class="col-lg-4 col-lg-offset-4 col-xs-12">
              <h2>Volume</h2>  
              <div class="row box">
                <div class="col-xs nogrow">
                  <svg class="icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24">
                    <polygon points="11 5 6 9 2 9 2 15 6 15 11 19 11 5"></polygon>
                    <path d="M19.07 4.93a10 10 0 0 1 0 14.14M15.54 8.46a5 5 0 0 1 0 7.07"></path>
                  </svg>
                </div>
                <div class="col-xs">
                  <div class="volume-container">
                    <input type="range" min="0" max="63" class="volume" id="volume-slider">
                  </div>
                </div>
              </div>
            </div>
          </div>
          
          <div class="row">
            <div class="col-lg-4 col-lg-offset-4 col-xs-12">
              <h2>Input</h2>
              <div class="row center-content nomargin" id="input-container">
                <!-- Inputs populated by JavaScript --> 
              </div>
            </div>
          </div>
        </div>

        <div id="page-settings" class="page">
          <!-- Settings Pages -->
          <div class="row">
            <div class="col-lg-4 col-lg-offset-4 col-xs-12">
              <h2>Settings</h2>
              <div class="tabs">
                <ul>
                  <li class="selected"><a href="#" data-tab="inputs">Inputs</a></li>
                  <li><a href="#" data-tab="system">System</a></li>
                  <li><a href="#" data-tab="wireless">Wireless</a></li>
                  <li><a href="#" data-tab="firmware">Firmware</a></li>
                </ul>
              </div>
            </div>
          </div>

          <div id="settings-content">
            <!-- Settings content populated by JavaScript -->
          </div>
        </div>
      </div>

      <!-- Modals and Overlays -->
      <div id="banner" style="display: none;">
        <div class="content">
          <h2>Connection lost</h2>
          <img src="img/connecting.svg">
          <p>Reconnecting</p>
        </div>
      </div>
    `;
  }

  setupEventListeners() {
    // Dark mode toggle
    const darkModeToggle = this.querySelector('#dark-mode-toggle');
    darkModeToggle?.addEventListener('click', (e) => {
      e.preventDefault();
      e.stopPropagation();
      const currentTheme = localStorage.getItem('theme') || 'light';
      const newTheme = currentTheme === 'light' ? 'dark' : 'light';
      this.toggleDarkMode(newTheme === 'dark');
    });

    // Home button (logo)
    const homeBtn = this.querySelector('#home-btn');
    homeBtn?.addEventListener('click', (e) => {
      e.preventDefault();
      e.stopPropagation();
      this.showPage('main');
    });

    // Settings button
    const settingsBtn = this.querySelector('#settings-btn');
    settingsBtn?.addEventListener('click', (e) => {
      e.preventDefault();
      e.stopPropagation();
      this.showPage('settings');
    });

    // Volume slider
    const volumeSlider = this.querySelector('#volume-slider');
    volumeSlider?.addEventListener('change', (e) => {
      this.updateVolume(parseInt(e.target.value));
    });

    // Tab navigation
    this.querySelectorAll('[data-tab]').forEach(tab => {
      tab.addEventListener('click', (e) => {
        e.preventDefault();
        e.stopPropagation();
        this.showSettingsTab(e.target.dataset.tab);
      });
    });

    // Hash change listener for routing
    window.addEventListener('hashchange', () => {
      this.handleRouteChange();
    });
  }

  initializeTheme() {
    const savedTheme = localStorage.getItem('theme') || 'light';
    document.documentElement.setAttribute('data-theme', savedTheme);
  }

  updateToggleState() {
    const savedTheme = localStorage.getItem('theme') || 'light';
    const button = this.querySelector('#dark-mode-toggle');
    
    if (!button) return;
    
    // Show the icon for the current mode
    if (savedTheme === 'dark') {
      // In dark mode, show moon
      button.innerHTML = `
        <svg class="icon icon-white" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24">
          <path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z"></path>
        </svg>
      `;
    } else {
      // In light mode, show sun
      button.innerHTML = `
        <svg class="icon icon-white" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24">
          <circle cx="12" cy="12" r="5"></circle>
          <line x1="12" y1="1" x2="12" y2="3"></line>
          <line x1="12" y1="21" x2="12" y2="23"></line>
          <line x1="4.22" y1="4.22" x2="5.64" y2="5.64"></line>
          <line x1="18.36" y1="18.36" x2="19.78" y2="19.78"></line>
          <line x1="1" y1="12" x2="3" y2="12"></line>
          <line x1="21" y1="12" x2="23" y2="12"></line>
          <line x1="4.22" y1="19.78" x2="5.64" y2="18.36"></line>
          <line x1="18.36" y1="5.64" x2="19.78" y2="4.22"></line>
        </svg>
      `;
    }
  }

  toggleDarkMode(isDark) {
    const theme = isDark ? 'dark' : 'light';
    document.documentElement.setAttribute('data-theme', theme);
    localStorage.setItem('theme', theme);
    this.updateToggleState();
  }

  handleInitialRoute() {
    const hash = window.location.hash.substring(1); // Remove #
    const [page, tab] = hash.split('/');
    
    if (page === 'settings') {
      this.showPage('settings');
      if (tab) {
        this.showSettingsTab(tab);
      }
    } else {
      this.showPage('main');
    }
  }

  handleRouteChange() {
    const hash = window.location.hash.substring(1); // Remove #
    const [page, tab] = hash.split('/');
    
    if (page === 'settings') {
      this.showPage('settings');
      if (tab) {
        this.showSettingsTab(tab);
      }
    } else {
      this.showPage('main');
    }
  }

  showPage(page) {
    // Hide all pages
    this.querySelectorAll('.page').forEach(p => p.classList.remove('active'));
    
    // Show requested page
    const targetPage = this.querySelector(`#page-${page}`);
    if (targetPage) {
      targetPage.classList.add('active');
      this.currentPage = page;
      
      // Update URL hash
      if (page === 'settings') {
        // Only update hash if not already on a settings tab
        if (!window.location.hash.startsWith('#settings/')) {
          window.location.hash = '#settings/inputs';
        }
      } else {
        window.location.hash = '';
      }
    }
  }

  showSettingsTab(tab) {
    // Update tab selection
    this.querySelectorAll('.tabs li').forEach(li => li.classList.remove('selected'));
    const activeTab = this.querySelector(`[data-tab="${tab}"]`)?.parentElement;
    if (activeTab) {
      activeTab.classList.add('selected');
    }

    // Update URL to include tab
    window.location.hash = `#settings/${tab}`;

    // Show tab content
    const settingsContent = this.querySelector('#settings-content');
    if (settingsContent) {
      switch (tab) {
        case 'inputs':
          this.renderInputSettings(settingsContent);
          break;
        case 'system':
          this.renderSystemSettings(settingsContent);
          break;
        case 'wireless':
          this.renderWirelessSettings(settingsContent);
          break;
        case 'firmware':
          this.renderFirmwareSettings(settingsContent);
          break;
      }
    }
  }

  renderInputSettings(container) {
    container.innerHTML = `
      <div class="row">
        <div class="col-lg-4 col-lg-offset-4 col-xs-12">
          <p>Configure input names and settings</p>
          <div id="input-settings-list">
            <!-- Input settings populated by JavaScript -->
          </div>
        </div>
      </div>
    `;
    this.updateInputSettings();
  }

  renderSystemSettings(container) {
    const settings = this.sysStatus.settings || {};
    container.innerHTML = `
      <div class="row">
        <div class="col-lg-6 col-lg-offset-4 col-xs-12">
          <p>Change system settings that affect the display and audio</p>
          
          <fieldset>
            <h3>Dim screen</h3>
            <p class="subtitle">Automatically dims the screen to a lower level after 10 seconds</p>
            <label class="switch">
              <input type="checkbox" id="settings-dim" ${settings.dim ? 'checked' : ''}>
              <span class="slider round"></span>
            </label>
          </fieldset>
          
          <fieldset>
            <h3>Absolute Volume</h3>
            <p class="subtitle">Show the actual volume level instead of a percentage</p>
            <label class="switch">
              <input type="checkbox" id="settings-absolute-vol" ${settings.absoluteVol ? 'checked' : ''}>
              <span class="slider round"></span>
            </label>
          </fieldset>
          
          <fieldset>
            <h3>Factory reset</h3>
            <p class="subtitle">Reset the device back to factory defaults</p>
            <button class="button danger" id="factory-reset-btn">Factory reset</button>
          </fieldset>
        </div>
      </div>
    `;
    
    // Setup system settings event listeners
    this.querySelector('#settings-dim')?.addEventListener('change', (e) => {
      this.updateSetting('dim', e.target.checked ? 1 : 0);
    });
    
    this.querySelector('#settings-absolute-vol')?.addEventListener('change', (e) => {
      this.updateSetting('absoluteVol', e.target.checked ? 1 : 0);
    });
  }

  renderWirelessSettings(container) {
    container.innerHTML = `
      <div class="row">
        <div class="col-lg-4 col-lg-offset-4 col-xs-12">
          <p>Connect to a wireless network</p>
          <div id="network-list">
            <p>Scanning for networks...</p>
          </div>
        </div>
      </div>
    `;
    this.scanNetworks();
  }

  renderFirmwareSettings(container) {
    container.innerHTML = `
      <div class="row">
        <div class="col-lg-4 col-lg-offset-4 col-xs-12">
          <p>Update firmware and system information</p>
          
          <div class="message">
            <div class="heading">
              <h3>Firmware Update</h3>
            </div>
            <p>Select a firmware file to update your device</p>
            <div class="firmware-upload-form">
              <input type="file" id="update-file" accept=".bin">
              <button class="button" id="update-btn">Update</button>
            </div>
          </div>
          
          <div id="progress-container" class="hidden">
            <p>Uploading...</p>
            <progress id="update-progress" value="0" max="100"></progress>
            <span id="update-percentage">0</span>%
          </div>
        </div>
      </div>
    `;
    
    // Setup firmware update
    this.querySelector('#update-btn')?.addEventListener('click', () => {
      this.uploadFirmware();
    });
  }

  async loadSystemStatus() {
    try {
      console.log('Loading system status...');
      const response = await fetch('/api/status');
      console.log('Response status:', response.status);
      this.sysStatus = await response.json();
      console.log('Loaded sysStatus:', this.sysStatus);
      console.log('Volume data:', this.sysStatus.volume);
      this.updateUI();
    } catch (error) {
      console.error('Failed to load system status:', error);
    }
  }

  updateUI() {
    // Update volume slider
    const volumeSlider = this.querySelector('#volume-slider');
    console.log('updateUI: volumeSlider found:', !!volumeSlider);
    console.log('updateUI: sysStatus.volume:', this.sysStatus.volume);
    if (volumeSlider && this.sysStatus.volume) {
      console.log('Setting volume slider to:', this.sysStatus.volume.current);
      volumeSlider.value = this.sysStatus.volume.current;
      volumeSlider.max = this.sysStatus.volume.max;
      console.log('Volume slider value is now:', volumeSlider.value);
    }

    // Update inputs
    this.updateInputs();
  }

  updateInputs() {
    const container = this.querySelector('#input-container');
    if (!container || !this.sysStatus.inputs) return;

    container.innerHTML = this.sysStatus.inputs
      .filter(input => input.enabled)
      .map(input => `
        <div class="col-xs-6 col-md-3">
          <label class="input-selector pointer">
            <span>${input.name}</span>
            <input type="radio" name="input" value="${input.id}" ${input.selected ? 'checked' : ''}>
            <div class="selector"></div>
          </label>
        </div>
      `).join('');

    // Setup input selection
    container.querySelectorAll('input[name="input"]').forEach(input => {
      input.addEventListener('change', (e) => {
        this.selectInput(parseInt(e.target.value));
      });
    });
  }

  async updateVolume(volume) {
    try {
      await fetch('/api/volume', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ volume })
      });
    } catch (error) {
      console.error('Failed to update volume:', error);
    }
  }

  async selectInput(inputId) {
    try {
      await fetch('/api/input', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ input: inputId })
      });
    } catch (error) {
      console.error('Failed to select input:', error);
    }
  }

  async updateSetting(setting, value) {
    try {
      await fetch(`/api/settings/${setting}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ state: value })
      });
    } catch (error) {
      console.error(`Failed to update ${setting}:`, error);
    }
  }

  async scanNetworks() {
    try {
      await fetch('/api/networks');
      // Networks will be updated via EventSource
    } catch (error) {
      console.error('Failed to scan networks:', error);
    }
  }

  connectEventSource() {
    if (!window.EventSource) return;

    this.eventSource = new EventSource('/events');
    
    this.eventSource.addEventListener('message', (e) => {
      try {
        const data = JSON.parse(e.data);
        this.handleRealtimeUpdate(data);
      } catch (error) {
        console.error('Failed to parse event data:', error);
      }
    });

    this.eventSource.addEventListener('error', () => {
      this.showConnectionLost();
    });
  }

  handleRealtimeUpdate(data) {
    // Handle real-time updates from the server
    if (data.volume !== undefined) {
      const slider = this.querySelector('#volume-slider');
      if (slider) slider.value = data.volume;
    }

    if (data.input !== undefined) {
      this.updateInputSelection(data.input);
    }
  }

  showConnectionLost() {
    const banner = this.querySelector('#banner');
    if (banner) {
      banner.style.display = 'flex';
    }
  }

  hideConnectionLost() {
    const banner = this.querySelector('#banner');
    if (banner) {
      banner.style.display = 'none';
    }
  }

  async uploadFirmware() {
    const fileInput = this.querySelector('#update-file');
    const file = fileInput?.files[0];
    
    if (!file) {
      alert('Please select a firmware file');
      return;
    }

    const formData = new FormData();
    formData.append('firmware', file);

    try {
      const response = await fetch('/update', {
        method: 'POST',
        body: formData
      });

      if (response.ok) {
        alert('Firmware updated successfully!');
      } else {
        alert('Firmware update failed');
      }
    } catch (error) {
      console.error('Firmware update error:', error);
      alert('Firmware update failed');
    }
  }
}

// Register the custom element
customElements.define('preamp-controller', PreampController);