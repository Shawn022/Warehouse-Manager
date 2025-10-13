import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import { Routes, Route } from 'react-router-dom'
import './App.css'
import Navbar from './components/navBar.jsx'
import Dashboard from './components/dashboard.jsx'
import Orders from './components/orders.jsx'
import Inventory from './components/inventory.jsx'
import Logistics from './components/logistics.jsx'
import Map from './components/map.jsx'
import { BrowserRouter } from 'react-router-dom'

createRoot(document.getElementById('root')).render(
  <StrictMode>
    <BrowserRouter>
      <Navbar />
      <div className="main">
        <Routes>
          <Route path="/" element={<Dashboard />} />
          <Route path="/orders" element={<Orders />} />
          <Route path="/inventory" element={<Inventory />} />
          <Route path="/logistics" element={<Logistics />} />
          <Route path="/map" element={<Map />} />
        </Routes>
      </div>
    </BrowserRouter>

  </StrictMode>
)

