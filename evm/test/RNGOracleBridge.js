const { expect } = require("chai");
const { ethers } = require("hardhat");
const ONE_TLOS = ethers.utils.parseEther("1.0");
const HALF_TLOS = ethers.utils.parseEther("0.5");
const MAX_REQUESTS = 10;
const GAS_PRICE = 499809179185;

describe("RNGOracleBridge Contract", function () {
    let bridge, owner, oracle, oracle2, user, consumer, gasOracle;
    beforeEach(async () => {
        [owner, oracle, oracle2, user] = await ethers.getSigners();
        let Consumer = await ethers.getContractFactory("RNGOracleConsumer");
        let Bridge = await ethers.getContractFactory("RNGOracleBridge");
        let GasOracle = await ethers.getContractFactory("GasOracleBridge");
        gasOracle = await GasOracle.deploy(owner.address, GAS_PRICE)
        bridge = await Bridge.deploy(ONE_TLOS, MAX_REQUESTS, oracle.address, gasOracle.address);
        consumer = await Consumer.deploy(bridge.address);
    })
    describe(":: Settings", function () {
        it("Should allow owner to set the fee" , async function () {
            await expect(bridge.setFee(HALF_TLOS)).to.not.be.reverted;
            expect(await bridge.fee()).to.equal(HALF_TLOS);
        });
        it("Should allow owner to set the fee" , async function () {
            await expect(bridge.setOracleEVMContract("0x6c33bdd2622e59fd10b411ff8d8d8d4dc5caf6c1")).to.not.be.reverted;
            let cc = await bridge.oracle_evm_contract();
            expect(cc.toLowerCase()).to.equal("0x6c33bdd2622e59fd10b411ff8d8d8d4dc5caf6c1");
        });
        it("Shouldn't allow addresses other than owner to set the fee" , async function () {
            await expect(bridge.connect(user).setFee(HALF_TLOS)).to.be.reverted;
        });
        it("Should allow owner to set the max requests" , async function () {
            await expect(bridge.setMaxRequests(MAX_REQUESTS / 2)).to.not.be.reverted;
            expect(await bridge.maxRequests()).to.equal(MAX_REQUESTS / 2);
        });
        it("Shouldn't allow addresses other than owner to set the max requests" , async function () {
            await expect(bridge.connect(user).setMaxRequests(MAX_REQUESTS)).to.be.reverted;
        });
    });
    describe(":: Getters", function () {
        it("Should be able to return the fee" , async function () {
            expect(await bridge.fee()).to.equal(ONE_TLOS);
        });
        it("Should be able to return the max requests" , async function () {
            expect(await bridge.maxRequests()).to.equal(MAX_REQUESTS);
        });
    });
    describe(":: Request", function () {
        it("Should be created with correct parameters" , async function () {
            let cost = await bridge.getCost(41000);
            await expect( consumer.makeRequest("120000", 10, 120, 41000, {"value": cost})).to.not.be.reverted;
        });
        it("Should revert if value is incorrect" , async function () {
            let cost = await bridge.getCost(20000);
            await expect(consumer.makeRequest("120000", 10, 120, 20000, {"value": cost.div(2)})).to.be.reverted;
        });
        it("Should be able to delete a request" , async function () {
            let cost = await bridge.getCost(20000);
            await expect( consumer.makeRequest("120000", 10, 120, 20000, {"value": cost })).to.not.be.reverted;
            await expect(bridge.deleteRequest(0)).to.not.be.reverted;
        });
        it("Should not be possible to have more than " + MAX_REQUESTS + " requests" , async function () {
            let cost = await bridge.getCost(20000);
            for(var i = 0; i < MAX_REQUESTS; i++){
                await expect(consumer.makeRequest("120000", 10, 120, 20000, {"value": cost })).to.not.be.reverted;
            }
            await expect(consumer.makeRequest("120000", 10, 120, 20000, {"value": cost })).to.be.reverted;
        });
    });
    describe(":: Response", function () {
        it("Shouldn't be able to reply from another address than the Request oracle" , async function () {
            let cost = await bridge.getCost(20000);
            await expect(consumer.makeRequest("120000", 10, 120, 20000, {"value": cost })).to.not.be.reverted;
            await expect(bridge.connect(user).reply(0, 116)).to.be.reverted;
        });
        it("Should not be able to reply to a Request without enough gas" , async function () {
            let cost = await bridge.getCost(1000);
            await expect(consumer.makeRequest("120000", 10, 120, 1000, {"value": cost })).to.not.be.reverted;
            await expect(bridge.connect(oracle).reply(0, 116)).to.be.reverted;
        });
        it("Should be able to reply to a Request" , async function () {
            let cost = await bridge.getCost(50000);
            await expect(consumer.makeRequest("120000", 10, 120, 50000, {"value": cost })).to.not.be.reverted;
            await expect(bridge.connect(oracle).reply(0, 116)).to.not.be.reverted;
        });
    });
});
